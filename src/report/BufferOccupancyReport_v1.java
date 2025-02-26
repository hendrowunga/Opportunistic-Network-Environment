package report;

import core.*; // Mengimpor kelas-kelas inti dari framework simulasi ONE

import java.util.*; // Mengimpor kelas-kelas utilitas Java seperti List, Map, ArrayList, HashMap, dll.

/**
 * Kelas ini menghasilkan laporan tentang penggunaan buffer setiap node dalam simulasi DTN.
 * Laporan ini mencatat snapshot penggunaan buffer pada interval waktu tertentu,
 * dan memberikan laporan akhir yang menunjukkan penggunaan buffer setiap node di akhir simulasi.
 */
public class BufferOccupancyReport_v1 extends Report implements UpdateListener {

	/** Pengaturan untuk interval pelaporan penggunaan buffer (dalam detik).
	 *  Pengguna dapat mengubah nilai ini dalam file konfigurasi simulasi (settings.txt). */
	public static final String BUFFER_REPORT_INTERVAL = "occupancyInterval";

	/** Nilai default untuk interval pelaporan (5 detik) jika pengaturan tidak ditemukan dalam file konfigurasi. */
	public static final int DEFAULT_BUFFER_REPORT_INTERVAL = 5;

	/** Waktu terakhir laporan dicatat (digunakan untuk mengatur interval).
	 *  Diinisialisasi dengan nilai minimum Double agar laporan pertama dicatat pada awal simulasi. */
	private double lastRecord = Double.MIN_VALUE;

	/** Interval waktu antara laporan (dalam detik).
	 *  Nilai ini dibaca dari file konfigurasi atau menggunakan nilai default. */
	private int interval;

	/** Riwayat penggunaan buffer untuk setiap node.
	 *  Map ini menyimpan serangkaian nilai penggunaan buffer dari waktu ke waktu untuk setiap node.
	 *  Kunci: Objek DTNHost (node).
	 *  Nilai: List<Double> yang berisi serangkaian nilai penggunaan buffer. */
	private Map<DTNHost, List<Double>> bufferOccupancyHistory = new HashMap<>();

	/**
	 * Membuat instance baru dari BufferOccupancy_v1.
	 * Membaca pengaturan interval pelaporan dari file konfigurasi.
	 */
	public BufferOccupancyReport_v1() {
		super(); // Memanggil konstruktor dari superclass (Report)

		Settings settings = getSettings(); // Mendapatkan objek Settings untuk membaca konfigurasi
		interval = settings.getInt(BUFFER_REPORT_INTERVAL, DEFAULT_BUFFER_REPORT_INTERVAL);
		// Membaca nilai interval dari pengaturan. Jika tidak ada, gunakan nilai default.
	}

	/**
	 * Method yang dipanggil setiap kali ada pembaruan dalam simulasi.
	 * Mencatat penggunaan buffer jika sudah waktunya untuk membuat laporan baru.
	 * @param hosts Daftar semua host (node) dalam simulasi.
	 */
	@Override
	public void updated(List<DTNHost> hosts) {
		// Memeriksa apakah sudah waktunya untuk membuat laporan berdasarkan interval waktu
		if (SimClock.getTime() - lastRecord >= interval) {
			lastRecord = SimClock.getTime(); // Memperbarui waktu terakhir laporan dicatat
			recordBufferOccupancy(hosts); // Memanggil method untuk mencatat penggunaan buffer setiap node
		}
	}

	/**
	 * Mencatat penggunaan buffer setiap node dalam daftar.
	 * @param hosts Daftar semua host dalam simulasi.
	 */
	private void recordBufferOccupancy(List<DTNHost> hosts) {
		for (DTNHost host : hosts) { // Iterasi melalui setiap host dalam daftar
			double occupancy = Math.min(host.getBufferOccupancy(), 100.0);
			// Mendapatkan penggunaan buffer host dan memastikan tidak lebih dari 100%

			List<Double> history = bufferOccupancyHistory.getOrDefault(host, new ArrayList<>());
			// Mendapatkan daftar penggunaan buffer yang ada untuk host ini.
			// Jika host belum ada di map, buat ArrayList baru.

			history.add(occupancy);
			// Menambahkan penggunaan buffer saat ini ke daftar

			bufferOccupancyHistory.put(host, history); // Store buffer occupancy for each host
			// Memperbarui map bufferOccupancyHistory dengan daftar yang diperbarui
		}
	}

	/**
	 * Method yang dipanggil ketika simulasi selesai.
	 * Menghasilkan dan menulis laporan akhir ke file output.
	 */
	@Override
	public void done() {
		write("Buffer Occupancy report for scenario " + getScenarioName() + "\n");
		// Menulis header laporan dengan nama skenario

		write("sim_time: " + format(getSimTime()) );
		// Menulis waktu simulasi
		write("====================================");
		write(String.format("%-10s | %-16s", "Host", "Buffer Occupancy (%)"));
		// Menulis header kolom
		write("------------------------------------");

		List<DTNHost> hosts = SimScenario.getInstance().getHosts();
		// Mendapatkan daftar semua host dalam simulasi

		TreeMap<DTNHost, List<Double>> sortedOccupancy = new TreeMap<>(Comparator.comparingInt(DTNHost::getAddress));
		// Membuat TreeMap untuk menyimpan data penggunaan buffer, diurutkan berdasarkan alamat host
		// TreeMap digunakan untuk memastikan bahwa laporan selalu dicetak dalam urutan host yang sama

		for (DTNHost host : hosts) {
			sortedOccupancy.put(host, bufferOccupancyHistory.getOrDefault(host, new ArrayList<>()));
		}
		// Mengisi TreeMap dengan data penggunaan buffer dari bufferOccupancyHistory

		try {
			// Melakukan iterasi melalui TreeMap dan menulis laporan
			for (Map.Entry<DTNHost, List<Double>> entry : sortedOccupancy.entrySet()) {
				DTNHost host = entry.getKey();
				// Mendapatkan host

				List<Double> occupancies = entry.getValue();
				// Mendapatkan daftar penggunaan buffer untuk host ini

				double finalOccupancy = 0.0;
				// Variabel untuk menyimpan penggunaan buffer terakhir

				if (!occupancies.isEmpty()) {
					finalOccupancy = occupancies.get(occupancies.size() - 1); // Ambil nilai terakhir sebagai penggunaan akhir
				}
				// Jika ada data penggunaan buffer, ambil nilai terakhir (yang merupakan penggunaan buffer terakhir)

				write(String.format("%-10s | %16.2f%%", host, finalOccupancy));
				// Menulis baris ke laporan yang menunjukkan ID host dan penggunaan buffer terakhir
			}
			write("------------------------------------");
			// Menulis pemisah
		} catch (Exception e) {
			write("Terjadi kesalahan saat mencetak laporan: " + e.getMessage());
			// Menulis pesan kesalahan ke laporan
			e.printStackTrace(); // Print stack trace untuk mempermudah debugging
		}
		write("\nDetailed Buffer Occupancy History:\n");

		for (Map.Entry<DTNHost, List<Double>> entry : bufferOccupancyHistory.entrySet()) {
			write("Node " + entry.getKey() + ": " + entry.getValue() + "\n");
		}
		// Menulis riwayat penggunaan buffer terperinci untuk setiap node

		super.done(); // Memanggil method done() dari superclass (Report)
	}
}