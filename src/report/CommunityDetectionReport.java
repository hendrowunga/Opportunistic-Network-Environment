package report;

import core.DTNHost;
import core.SimScenario;
import routing.DecisionEngineRouter;
import routing.MessageRouter;
import routing.RoutingDecisionEngine;
import routing.community.CommunityDetectionEngine;

import java.util.LinkedList;
import java.util.List;
import java.util.Set;

/**
 * <p>Reports the local communities at each node whenever the done() method is
 * called. Only those nodes whose router is a DecisionEngineRouter and whose
 * RoutingDecisionEngine implements the
 * routing.community.CommunityDetectionEngine are reported. In this way, the
 * report is able to output the result of any of the community detection
 * algorithms.</p>
 *
 * @author PJ Dillon, University of Pittsburgh
 */
public class CommunityDetectionReport extends Report
{
    public CommunityDetectionReport()
    {
        init();
    }

    @Override
    public void done()
    {
        List<DTNHost> nodes = SimScenario.getInstance().getHosts();
        List<Set<DTNHost>> communities = new LinkedList<Set<DTNHost>>();

        for(DTNHost h : nodes)
        {
            MessageRouter r = h.getRouter();
            if(!(r instanceof DecisionEngineRouter) )
                continue;
            RoutingDecisionEngine de = ((DecisionEngineRouter)r).getDecisionEngine();
            if(!(de instanceof CommunityDetectionEngine))
                continue;
            CommunityDetectionEngine cd = (CommunityDetectionEngine)de;

            boolean alreadyHaveCommunity = false;
            Set<DTNHost> nodeComm = cd.getLocalCommunity();

            // Test to see if another node already reported this community
            for(Set<DTNHost> c : communities)
            {
                if(c.containsAll(nodeComm) && nodeComm.containsAll(c))
                {
                    alreadyHaveCommunity = true;
                }
            }

            if(!alreadyHaveCommunity && nodeComm.size() > 0)
            {
                communities.add(nodeComm);
            }
        }

        // print each community and its size out to the file
        for(Set<DTNHost> c : communities)
            write("" + c.size() + ' ' + c);

        super.done();
    }


}
