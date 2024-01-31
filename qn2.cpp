#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

vector<int> successfulTransmissions;
int Totalcollisions = 0;

class Channel
{
public:
    set<int> channelNodes;

    bool isIdle()
    {
        return channelNodes.size() == 0;
    }

    bool isBusy()
    {
        return !isIdle();
    }

    void clear()
    {
        channelNodes.clear();
    }
};

class Node
{
public:
    int id;
    int backoffMax;
    int dataSendingTime;
    int backoff;
    bool transmitting;
    int timesent;
    int successfulAttempts;
    int backoffAttempts;
    int collisions;

    Node(int id, int backoffMax, int dataSendingTime) : id(id), backoffMax(backoffMax), dataSendingTime(dataSendingTime), backoff(0), transmitting(false), timesent(0), successfulAttempts(0), backoffAttempts(0), collisions(0) {}

    void Backoff()
    {
        timesent = 0;
        backoff = rand() % backoffMax + 1; // channel is busy
        cout << "Node " << id << " backed off by " << backoff << endl;
        backoffAttempts++;
    }

    void Collide()
    {
        cout << "Node " << id << " collided with other nodes " << endl;
        collisions++;
    }

    bool attemptTransmission(Channel &channel)
    {
        if (channel.channelNodes.size() == 1 and *channel.channelNodes.begin() == id)
        {
            return true;
        }
        if (timesent == dataSendingTime)
        {
            return false;
        }
        if (channel.isBusy())
        {
            Backoff();
            return false;
        }
        return true;
    }

    void transmit(Channel &channel)
    {
        channel.channelNodes.insert(id);
        timesent++;
        successfulAttempts++;
    }

    bool stillTransmitting()
    {
        return timesent < dataSendingTime and timesent > 0;
    }

    void decrementBackoff()
    {
        if (backoff > 0)
        {
            backoff--;
        }
    }
};

int main()
{
    srand(time(0));

    int numNodes = 3;        // Number of nodes in the network
    int maxBackoff = 10;     // Maximum backoff interval
    int numSlots = 50;       // Number of time slots to simulate
    int maxPacketLength = 4; // maximum length of data packet

    Channel oldchannel, channel;
    vector<Node> nodes;

    cout<<"Enter Number of Nodes in network: ";
    cin>>numNodes;

    cout<<"Do you wish to manually enter packet transmission length of each node (y/n)? ";
    char c;
    cin>>c;
    if (c=='y'){
        for (int i = 0; i < numNodes; i++)
        {
            int dataSendingTime;
            cout<<"Enter Data Sending Time for Node "<<i<<": ";
            cin>>dataSendingTime;
            nodes.push_back(Node(i, maxBackoff, dataSendingTime));
        }
    }

    else {
        cout<<"Data Sending Times:"<<endl;

        for (int i = 0; i < numNodes; i++)
        {
            int dataSendingTime = (rand() % maxPacketLength) + 1;
            nodes.push_back(Node(i, maxBackoff, dataSendingTime));
            cout << "Node "<<i << ": " << dataSendingTime << endl;
        }
    }



    for (int slot = 0; slot < numSlots; slot++)
    {
        cout << "\nTime Slot " << slot + 1 << " :" << endl;

        // Check if the old channel nodes are still transmitting

        set<int> oldChannel = oldchannel.channelNodes;

        for (auto &node : nodes)
        {
            for (auto &id : oldChannel)
            {
                if (id == node.id)
                {
                    if (!node.stillTransmitting())
                    {
                        if (oldchannel.channelNodes.find(id) != oldchannel.channelNodes.end())
                            oldchannel.channelNodes.erase(oldchannel.channelNodes.find(node.id));
                        successfulTransmissions.push_back(id);
                    }
                }
            }
        }

        cout << "Nodes of Prev transition still continuing :";

        for (auto node : oldchannel.channelNodes)
        {
            cout << node << " ";
        }
        cout << endl;

        // Trying for new nodes to attempt transition (can lead to backoff)
        for (auto &node : nodes)
        {
            if (node.backoff == 0)
            {
                if (node.attemptTransmission(oldchannel))
                    node.transmit(channel);
            }
        }

        cout << "Nodes of current transition : ";

        for (auto &node : channel.channelNodes)
        {
            cout << node << " ";
        }
        cout << endl;

        // Trying for new nodes to start transition (can lead to collisions)
        if (channel.channelNodes.size() > 1)
        {
            for (auto &node : nodes)
            {
                for (auto &id : channel.channelNodes)
                {
                    if (id == node.id)
                    {
                        node.Collide();
                        node.Backoff();
                    }
                }
            }
            Totalcollisions++;
            channel.clear();
        }
        oldchannel = channel;
        channel.clear();

        for (auto &node : nodes)
        {
            node.decrementBackoff();        //passing 1 unit of time
        }

    }

    cout<<endl<<endl<<"===========Report==========="<<endl<<endl;

    cout << "Nodes which Successfully Transmitted entire message: ";
    sort(successfulTransmissions.begin(),successfulTransmissions.end());
    for (auto &ele: successfulTransmissions){
        cout<<ele<<" ";
    }
    if (successfulTransmissions.size()==0){cout<<"None";}
    cout<<endl;
    cout<<"Number nodes which Successfully Transmitted entire message: "<<successfulTransmissions.size()<<endl;
    cout << "Collisions: " << Totalcollisions << endl;
    cout << "Backoff Attempts: " << endl;
    int sum=0;
    for (auto &node:nodes){
        cout<<"Node "<<node.id<<": "<<node.backoffAttempts<<endl;
        sum+=node.backoffAttempts;
    }
    cout<<"Total Number of Backoff Attempts: "<<sum<<endl<<endl;

    return 0;
}