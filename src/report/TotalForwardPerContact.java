package report;

import core.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class TotalForwardPerContact extends Report implements MessageListener,ConnectionListener, UpdateListener {

    private double lastRecord;
    private int interval;
    private Map<DTNHost, Integer> nrofForwards;
    private Map<Integer, Integer> nrofForwardRecords;
    private int nrofContacts;
    private static final String NROF_CONTACT_INTERVAL = "perTotalContact";
    private static final int DEFAULT_CONTACT_COUNT = 600;

    public TotalForwardPerContact() {
        init();
        if (getSettings().contains(NROF_CONTACT_INTERVAL)) {
            interval = getSettings().getInt(NROF_CONTACT_INTERVAL);
        } else {
            interval = DEFAULT_CONTACT_COUNT;
        }
    }

    @Override
    protected void init() {
        super.init();
        nrofForwards = new HashMap<>();
        this.lastRecord = 0;
        this.interval = 0;
        this.nrofContacts=0;
        nrofForwardRecords = new HashMap<>();
    }


    @Override
    public void done() {
        String output = "Contacts\tTotalForwards\n";
        for (Map.Entry<Integer, Integer> entry : nrofForwardRecords.entrySet()) {
            output += entry.getKey() + "\t" + entry.getValue() + "\n";
        }
        write(output);
        super.done();
    }


    @Override
    public void updated(List<DTNHost> hosts) {
        if (nrofContacts - lastRecord >= interval) {
            // Menghitung total forwards dengan cara sederhana (loop biasa)
            int totalForwardCount = 0;
            for (Integer count : nrofForwards.values()) {
                totalForwardCount += count; // Menjumlahkan semua forward dari setiap host
            }

            // Simpan hasil ke dalam record
            nrofForwardRecords.put(nrofContacts, totalForwardCount);

            // Update lastRecord ke jumlah total kontak saat ini
            lastRecord = nrofContacts;
        }
    }

    @Override
    public void newMessage(Message m) {

    }

    @Override
    public void messageTransferStarted(Message m, DTNHost from, DTNHost to) {

    }

    @Override
    public void messageDeleted(Message m, DTNHost where, boolean dropped) {

    }

    @Override
    public void messageTransferAborted(Message m, DTNHost from, DTNHost to) {

    }

    @Override
    public void messageTransferred(Message m, DTNHost from, DTNHost to, boolean firstDelivery) {
        if (firstDelivery) {
            // Tambahkan jumlah forward untuk host 'from'
            nrofForwards.put(from, nrofForwards.getOrDefault(from, 0) + 1);
        }
    }

    @Override
    public void hostsConnected(DTNHost host1, DTNHost host2) {
        nrofContacts++;

    }

    @Override
    public void hostsDisconnected(DTNHost host1, DTNHost host2) {

    }
}
