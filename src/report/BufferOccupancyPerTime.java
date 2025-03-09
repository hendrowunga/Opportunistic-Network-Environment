package report;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

import core.DTNHost;
import core.Settings;
import core.UpdateListener;

public class BufferOccupancyPerTime extends Report implements UpdateListener {
    public static final String BUFFER_REPORT_INTERVAL = "occupancyInterval";
    public static final int DEFAULT_BUFFER_REPORT_INTERVAL = 3600;

    private double lastRecord;
    private int interval;
    private Map<Integer, List<Double>> bufferCounts;

    public BufferOccupancyPerTime() {
        init();
        // Mengambil nilai interval dari pengaturan jika ada, jika tidak, gunakan default
        Settings settings = getSettings();
        if (settings.contains(BUFFER_REPORT_INTERVAL)) {
            this.interval = settings.getInt(BUFFER_REPORT_INTERVAL);
        } else {
            this.interval = DEFAULT_BUFFER_REPORT_INTERVAL;
        }
    }

    @Override
    protected void init() {
        super.init();
        this.lastRecord = Double.MIN_VALUE;
        this.bufferCounts = new HashMap<>();

    }

    public void updated(List<DTNHost> hosts) {
        double simTime = getSimTime();

        // Melewati proses update jika masih dalam tahap warmup
        if (isWarmup()) return;

        // Periksa apakah sudah waktunya merekam data baru berdasarkan interval
        if (simTime - lastRecord >= interval) {
            recordBufferOccupancy(hosts);
            lastRecord = simTime - simTime % interval;
        }
    }

    private void recordBufferOccupancy(List<DTNHost> hosts) {
        for (DTNHost h : hosts) {
            int nodeId = h.getAddress();
            double bufferUsage = Math.min(h.getBufferOccupancy(), 100.0);
            bufferCounts.computeIfAbsent(nodeId, k -> new ArrayList<>()).add(bufferUsage);
        }
    }

    @Override
    public void done() {
        List<Map.Entry<Integer, List<Double>>> sortedEntries = new ArrayList<>(bufferCounts.entrySet());

        // Sorting berdasarkan jumlah data
        if (sortedEntries.size() > 10_000) {
            // Jika data lebih dari 10.000, gunakan Collections.sort()
            Collections.sort(sortedEntries, Comparator.comparingInt(Map.Entry::getKey));
        } else {
            // Jika data kurang dari 10.000, gunakan Stream API untuk sorting yang lebih ringan
            sortedEntries = sortedEntries.stream()
                    .sorted(Comparator.comparingInt(Map.Entry::getKey))
                    .toList();
        }

        // Menuliskan hasil ke output
        for (Map.Entry<Integer, List<Double>> entry : sortedEntries) {
            StringBuilder line = new StringBuilder("Node ").append(entry.getKey()).append("\t");
            entry.getValue().forEach(buffer -> line.append(buffer).append("\t"));
            write(line.toString());
        }

        super.done();
    }
}
//        // Mengubah bufferCounts (HashMap<Integer, List<Double>>) menjadi list of entries agar bisa disortir.
//        List<Map.Entry<Integer, List<Double>>> sortedEntries = new ArrayList<>(bufferCounts.entrySet());
//
//        // Sorting list berdasarkan Node ID dalam urutan menaik (ascending).
//        // Map.Entry::getKey mengacu pada kunci (Node ID) dalam map.
//        sortedEntries.sort(Comparator.comparingInt(Map.Entry::getKey));
//
//        // Jika ingin mengubah ke urutan menurun (descending), gunakan baris berikut:
//        // sortedEntries.sort(Comparator.comparingInt(Map.Entry::getKey).reversed());
//
//        // Loop melalui setiap entry yang sudah diurutkan berdasarkan Node ID.
//        for (Map.Entry<Integer, List<Double>> entry : sortedEntries) {
//            // Membuat string yang dimulai dengan "Node <ID>\t".
//            StringBuilder line = new StringBuilder("Node ").append(entry.getKey()).append("\t");
//
//            // Menambahkan semua nilai buffer yang tersimpan dalam List<Double>.
//            for (Double buffer : entry.getValue()) {
//                line.append(buffer).append("\t"); // Menambahkan nilai buffer dengan tab sebagai pemisah.
//            }
//
//            // Menuliskan hasil akhir ke output (bisa berupa file/log console tergantung implementasi write()).
//            write(line.toString());
//        }
//
//        // Memanggil metode done() dari superclass untuk memastikan proses selesai dengan benar.
//        super.done();

