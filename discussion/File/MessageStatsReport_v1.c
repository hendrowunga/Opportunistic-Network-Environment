/*
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details.
 */
package report;

import core.*;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Report for generating different kind of total statistics about message
 * relaying performance. Messages that were created during the warm up period
 * are ignored.
 * <P><strong>Note:</strong> if some statistics could not be created (e.g.
 * overhead ratio if no messages were delivered) "NaN" is reported for
 * double values and zero for integer median(s).
 */
public class MessageStatsReport_V1 extends Report implements MessageListener, UpdateListener, ConnectionListener {
	private Map<String, Double> creationTimes;
	private List<Double> latencies;
	private List<Integer> hopCounts;
	private List<Double> msgBufferTime;
	private List<Double> rtt; // round trip times

	private int nrofDropped;
	private int nrofRemoved;
	private int nrofStarted;
	private int nrofAborted;
	private int nrofRelayed;
	private int nrofCreated;
	private int nrofResponseReqCreated;
	private int nrofResponseDelivered;
	private int nrofDelivered;

	private int totalContacts; // Tambahkan ini untuk melacak total kontak
	private boolean headerWritten = false; // Tambahkan ini

	private int lastIntervalCreated;
	private int lastIntervalRelayed;
	private int lastIntervalDelivered;
	private double lastIntervalLatencySum;
	private double lastCurrentTime; // Variable baru

	private double intervalLength = 60; // Panjang interval waktu dalam detik (dapat dikonfigurasi)

	/**
	 * Constructor.
	 */
	public MessageStatsReport_V1() {
		init();
	}

	@Override
	protected void init() {
		super.init();
		this.creationTimes = new HashMap<String, Double>();
		this.latencies = new ArrayList<Double>();
		this.msgBufferTime = new ArrayList<Double>();
		this.hopCounts = new ArrayList<Integer>();
		this.rtt = new ArrayList<Double>();

		this.nrofDropped = 0;
		this.nrofRemoved = 0;
		this.nrofStarted = 0;
		this.nrofAborted = 0;
		this.nrofRelayed = 0;
		this.nrofCreated = 0;
		this.nrofResponseReqCreated = 0;
		this.nrofResponseDelivered = 0;
		this.nrofDelivered = 0;

		this.totalContacts = 0; // Inisialisasi total kontak
		this.headerWritten = false; // Inisialisasi headerWritten

		this.lastIntervalCreated = 0;
		this.lastIntervalRelayed = 0;
		this.lastIntervalDelivered = 0;
		this.lastIntervalLatencySum = 0;
		this.lastCurrentTime = 0;
	}


	public void messageDeleted(Message m, DTNHost where, boolean dropped) {
		if (isWarmupID(m.getId())) {
			return;
		}

		if (dropped) {
			this.nrofDropped++;
		}
		else {
			this.nrofRemoved++;
		}

		this.msgBufferTime.add(getSimTime() - m.getReceiveTime());
	}


	public void messageTransferAborted(Message m, DTNHost from, DTNHost to) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofAborted++;
	}


	public void messageTransferred(Message m, DTNHost from, DTNHost to,
								   boolean finalTarget) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofRelayed++;
		if (finalTarget) {
			double latency = getSimTime() -
					this.creationTimes.get(m.getId()) ;
			this.latencies.add(latency);
			this.nrofDelivered++;
			this.hopCounts.add(m.getHops().size() - 1);

			if (m.isResponse()) {
				this.rtt.add(getSimTime() -  m.getRequest().getCreationTime());
				this.nrofResponseDelivered++;
			}
		}
	}


	public void newMessage(Message m) {
		if (isWarmup()) {
			addWarmupID(m.getId());
			return;
		}

		this.creationTimes.put(m.getId(), getSimTime());
		this.nrofCreated++;
		if (m.getResponseSize() > 0) {
			this.nrofResponseReqCreated++;
		}
	}


	public void messageTransferStarted(Message m, DTNHost from, DTNHost to) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofStarted++;
	}

	private int intervalContacts = 0; // Untuk menghitung kontak dalam interval
	@Override
	public void hostsConnected(DTNHost host1, DTNHost host2) {
		totalContacts++; // update total
		intervalContacts++; // Tambahkan penghitung interval
	}

	@Override
	public void hostsDisconnected(DTNHost host1, DTNHost host2) {
		//totalContacts--; // Jika Anda menghitung kontak unik, kurangi di sini
	}

	@Override
	public void updated(List<DTNHost> hosts) {
		double currentTime = getSimTime();
		if (currentTime >= lastCurrentTime + intervalLength) {

			System.out.println("Interval reached! Time: " + currentTime);

			// Hitung metrik interval
			int intervalCreatedCount = nrofCreated - lastIntervalCreated;
			int intervalRelayedCount = nrofRelayed - lastIntervalRelayed;
			int intervalDeliveredCount = nrofDelivered - lastIntervalDelivered;
			double intervalLatencySumValue = 0;

			// Calculate the sum of latencies
			for (Double latency : latencies) {
				intervalLatencySumValue += latency;
			}

			double deliveryProb = (intervalCreatedCount > 0) ? (100.0 * intervalDeliveredCount) / intervalCreatedCount : 0;
			double overHead = (intervalDeliveredCount > 0) ? (1.0 * (intervalRelayedCount - intervalDeliveredCount)) / intervalDeliveredCount : Double.NaN;
			double averageLatency = (intervalDeliveredCount > 0) ? intervalLatencySumValue / intervalDeliveredCount : 0;

			//Format The output
			if (!headerWritten) {

				write(String.format("%-15s\t%-15s\t%-15s\t%-15s\t%-15s\t%-15s%n",
						"Interval Stats", "Total Contacts", "Delivery Ratio", "Overhead Ratio", "Average Latency", "Total Forwards"));
				write("-------------------------------------------------------------------------------------------------------------------------\n");
				headerWritten = true;

			}
			// Format dan tulis metrik interval ke file laporan
			String intervalStats = String.format(
					"%-15.2f\t%-15d\t%-15.2f\t%-15.2f\t%-15.2f\t%-15d%n",
					currentTime,
					intervalContacts,
					deliveryProb,
					overHead,
					averageLatency,
					intervalRelayedCount
			);
			write(intervalStats);

			// Simpan nilai saat ini untuk perhitungan interval berikutnya
			lastIntervalCreated = nrofCreated;
			lastIntervalRelayed = nrofRelayed;
			lastIntervalDelivered = nrofDelivered;
			lastIntervalLatencySum = 0;
			lastCurrentTime = currentTime;
			intervalContacts = 0;


		}
	}

	@Override
	public void done() {
		write("Message stats for scenario " + getScenarioName() +
				"\nsim_time: " + format(getSimTime()));

		double deliveryProb = 0; // delivery probability
		double responseProb = 0; // request-response success probability
		double overHead = Double.NaN;  // overhead ratio

		if (this.nrofCreated > 0) {
			deliveryProb = (1.0 * this.nrofDelivered) / this.nrofCreated;
		}
		if (this.nrofDelivered > 0) {
			overHead = (1.0 * (this.nrofRelayed - this.nrofDelivered)) /
					this.nrofDelivered;
		}
		if (this.nrofResponseReqCreated > 0) {
			responseProb = (1.0* this.nrofResponseDelivered) /
					this.nrofResponseReqCreated;
		}

		String statsText = "created: " + this.nrofCreated +
				"\nstarted: " + this.nrofStarted +
				"\nrelayed: " + this.nrofRelayed +
				"\naborted: " + this.nrofAborted +
				"\ndropped: " + this.nrofDropped +
				"\nremoved: " + this.nrofRemoved +
				"\ndelivered: " + this.nrofDelivered +
				"\ndelivery_prob: " + format(deliveryProb) +
				"\nresponse_prob: " + format(responseProb) +
				"\noverhead_ratio: " + format(overHead) +
				"\nlatency_avg: " + getAverage(this.latencies) +
				"\nlatency_med: " + getMedian(this.latencies) +
				"\nhopcount_avg: " + getIntAverage(this.hopCounts) +
				"\nhopcount_med: " + getIntMedian(this.hopCounts) +
				"\nbuffertime_avg: " + getAverage(this.msgBufferTime) +
				"\nbuffertime_med: " + getMedian(this.msgBufferTime) +
				"\nrtt_avg: " + getAverage(this.rtt) +
				"\nrtt_med: " + getMedian(this.rtt)
				;

		write(statsText);
		super.done();
	}

//	@Override
//	public void hostsUpdated(List<DTNHost> hosts) {
//	}
}







---------------------------------
/*
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details.
 */
package report;

import core.*;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Report for generating different kind of total statistics about message
 * relaying performance. Messages that were created during the warm up period
 * are ignored.
 * <P><strong>Note:</strong> if some statistics could not be created (e.g.
 * overhead ratio if no messages were delivered) "NaN" is reported for
 * double values and zero for integer median(s).
 */
public class MessageStatsReport_V1 extends Report implements MessageListener, UpdateListener, ConnectionListener {
	private Map<String, Double> creationTimes;
	private List<Double> latencies;
	private List<Integer> hopCounts;
	private List<Double> msgBufferTime;
	private List<Double> rtt; // round trip times

	private int nrofDropped;
	private int nrofRemoved;
	private int nrofStarted;
	private int nrofAborted;
	private int nrofRelayed;
	private int nrofCreated;
	private int nrofResponseReqCreated;
	private int nrofResponseDelivered;
	private int nrofDelivered;

	private int totalContacts; // Tambahkan ini untuk melacak total kontak

	private double intervalLength = 60; // Panjang interval waktu dalam detik (dapat dikonfigurasi)

	// Struktur data untuk menyimpan data interval
	private List<IntervalData> intervalDataList = new ArrayList<>();

	/**
	 * Constructor.
	 */
	public MessageStatsReport_V1() {
		init();
	}

	@Override
	protected void init() {
		super.init();
		this.creationTimes = new HashMap<String, Double>();
		this.latencies = new ArrayList<Double>();
		this.msgBufferTime = new ArrayList<Double>();
		this.hopCounts = new ArrayList<Integer>();
		this.rtt = new ArrayList<Double>();

		this.nrofDropped = 0;
		this.nrofRemoved = 0;
		this.nrofStarted = 0;
		this.nrofAborted = 0;
		this.nrofRelayed = 0;
		this.nrofCreated = 0;
		this.nrofResponseReqCreated = 0;
		this.nrofResponseDelivered = 0;
		this.nrofDelivered = 0;

		this.totalContacts = 0; // Inisialisasi total kontak

		this.intervalDataList.clear(); // Inisialisasi list
	}


	public void messageDeleted(Message m, DTNHost where, boolean dropped) {
		if (isWarmupID(m.getId())) {
			return;
		}

		if (dropped) {
			this.nrofDropped++;
		}
		else {
			this.nrofRemoved++;
		}

		this.msgBufferTime.add(getSimTime() - m.getReceiveTime());
	}


	public void messageTransferAborted(Message m, DTNHost from, DTNHost to) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofAborted++;
	}


	public void messageTransferred(Message m, DTNHost from, DTNHost to,
								   boolean finalTarget) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofRelayed++;
		if (finalTarget) {
			double latency = getSimTime() -
					this.creationTimes.get(m.getId()) ;
			this.latencies.add(latency);
			this.nrofDelivered++;
			this.hopCounts.add(m.getHops().size() - 1);

			if (m.isResponse()) {
				this.rtt.add(getSimTime() -  m.getRequest().getCreationTime());
				this.nrofResponseDelivered++;
			}
		}
	}


	public void newMessage(Message m) {
		if (isWarmup()) {
			addWarmupID(m.getId());
			return;
		}

		this.creationTimes.put(m.getId(), getSimTime());
		this.nrofCreated++;
		if (m.getResponseSize() > 0) {
			this.nrofResponseReqCreated++;
		}
	}


	public void messageTransferStarted(Message m, DTNHost from, DTNHost to) {
		if (isWarmupID(m.getId())) {
			return;
		}

		this.nrofStarted++;
	}

	@Override
	public void hostsConnected(DTNHost host1, DTNHost host2) {
		totalContacts++; // update total
	}

	@Override
	public void hostsDisconnected(DTNHost host1, DTNHost host2) {
		//totalContacts--; // Jika Anda menghitung kontak unik, kurangi di sini
	}

	@Override
	public void updated(List<DTNHost> hosts) {
		System.out.println("Metode updated() dipanggil. Waktu: " + getSimTime());
		double currentTime = getSimTime();
		if (currentTime >=0  && (currentTime % intervalLength) <1) {
			// Hitung metrik
			double deliveryProb = (this.nrofCreated > 0) ? (100.0 * this.nrofDelivered) / this.nrofCreated : 0;
			double overHead = (this.nrofDelivered > 0) ? (1.0 * (this.nrofRelayed - this.nrofDelivered)) / this.nrofDelivered : Double.NaN;
			double averageLatency = 0;
			if(this.nrofDelivered > 0){
				double totalLatencyValue = 0;
				for (Double latency : latencies) {
					totalLatencyValue += latency;
				}
				averageLatency = totalLatencyValue / this.nrofDelivered;
			}
			// Buat objek IntervalData
			IntervalData data = new IntervalData(currentTime, totalContacts, deliveryProb, overHead, averageLatency, nrofRelayed);

			// Tambahkan ke list
			intervalDataList.add(data);
			System.out.println("Menambahkan data interval: " + data.currentTime + "Jumlah data :" + intervalDataList.size()); // Debugging
		}
	}

	@Override
	public void done() {
		write("Message stats for scenario " + getScenarioName() +
				"\nsim_time: " + format(getSimTime()));

		// Tulis header
		write(String.format("%-15s\t%-15s\t%-15s\t%-15s\t%-15s\t%-15s%n",
				"Interval Stats", "Total Contacts", "Delivery Ratio", "Overhead Ratio", "Average Latency", "Total Forwards"));
		write("-------------------------------------------------------------------------------------------------------------------------\n");

		//Tulis data interval
		for (IntervalData data : intervalDataList) {
			write(String.format(
					"%-15.2f\t%-15d\t%-15.2f\t%-15.2f\t%-15.2f\t%-15d%n",
					data.currentTime,
					data.totalContacts,
					data.deliveryRatio,
					data.overheadRatio,
					data.averageLatency,
					data.totalForwards
			));
		}
		// Pastikan Loop di eksekusi
		System.out.println("Jumlah Interval Data: " + intervalDataList.size());

		double deliveryProb = 0; // delivery probability
		double responseProb = 0; // request-response success probability
		double overHead = Double.NaN;  // overhead ratio

		if (this.nrofCreated > 0) {
			deliveryProb = (1.0 * this.nrofDelivered) / this.nrofCreated;
		}
		if (this.nrofDelivered > 0) {
			overHead = (1.0 * (this.nrofRelayed - this.nrofDelivered)) /
					this.nrofDelivered;
		}
		if (this.nrofResponseReqCreated > 0) {
			responseProb = (1.0* this.nrofResponseDelivered) /
					this.nrofResponseReqCreated;
		}

		String statsText = "created: " + this.nrofCreated +
				"\nstarted: " + this.nrofStarted +
				"\nrelayed: " + this.nrofRelayed +
				"\naborted: " + this.nrofAborted +
				"\ndropped: " + this.nrofDropped +
				"\nremoved: " + this.nrofRemoved +
				"\ndelivered: " + this.nrofDelivered +
				"\ndelivery_prob: " + format(deliveryProb) +
				"\nresponse_prob: " + format(responseProb) +
				"\noverhead_ratio: " + format(overHead) +
				"\nlatency_avg: " + getAverage(this.latencies) +
				"\nlatency_med: " + getMedian(this.latencies) +
				"\nhopcount_avg: " + getIntAverage(this.hopCounts) +
				"\nhopcount_med: " + getIntMedian(this.hopCounts) +
				"\nbuffertime_avg: " + getAverage(this.msgBufferTime) +
				"\nbuffertime_med: " + getMedian(this.msgBufferTime) +
				"\nrtt_avg: " + getAverage(this.rtt) +
				"\nrtt_med: " + getMedian(this.rtt)
				;

		write(statsText);
		super.done();
	}


	// Class untuk data interval
	private static class IntervalData {
		public double currentTime;
		public int totalContacts;
		public double deliveryRatio;
		public double overheadRatio;
		public double averageLatency;
		public int totalForwards;

		public IntervalData(double currentTime, int totalContacts, double deliveryRatio, double overheadRatio, double averageLatency, int totalForwards) {
			this.currentTime = currentTime;
			this.totalContacts = totalContacts;
			this.deliveryRatio = deliveryRatio;
			this.overheadRatio = overheadRatio;
			this.averageLatency = averageLatency;
			this.totalForwards = totalForwards;
		}
	}
}
-----------------------------------
