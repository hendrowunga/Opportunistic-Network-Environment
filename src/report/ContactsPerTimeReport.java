package report;

import core.ConnectionListener;
import core.DTNHost;
import core.SimClock;

import java.util.Iterator;
import java.util.LinkedList;

public class ContactsPerTimeReport extends Report implements ConnectionListener {
    private LinkedList<Integer> contactCounts;
    private int currentHourCount;
    private int currentHour;

    public ContactsPerTimeReport() {
        init();
    }

    @Override
    public void init() {
        super.init();
        contactCounts = new LinkedList<Integer>();
    }

    public void hostsConnected(DTNHost host1, DTNHost host2) {
        int time = SimClock.getIntTime() / 3600;
        while (Math.floor(time) > currentHour) {
            contactCounts.add( Integer.valueOf(currentHourCount));
            currentHourCount = 0;
            currentHour++;
        }

        currentHourCount++;
    }

    public void hostsDisconnected(DTNHost host1, DTNHost host2) {
        // Do nothing
    }

    public void done() {
        Iterator<Integer> iterator = contactCounts.iterator();
        int hour = 0;
        while (iterator.hasNext()) {
            Integer count = (Integer)iterator.next();
            write(hour + "\t" + count);
            hour++;
        }
        super.done();
    }
}
