#include <boost/signals2.hpp>
#include <iostream>
#include <string>

struct NewsAgency
{
    using SignalType = boost::signals2::signal<void(const std::string &)>;

    SignalType news_alert;

    void publish(const std::string &headline)
    {
        std::cout << "Publishing: " << headline << '\n';
        news_alert(headline);
    }
};

class Subscriber
{
private:
    std::string name;

public:
    explicit Subscriber(std::string n) : name(std::move(n)) {}

    void receive(const std::string &headline)
    {
        std::cout << "  [" << name << "] received: " << headline << '\n';
    }
};

int main()
{
    NewsAgency agency;

    Subscriber alice("Alice");
    Subscriber bob("Bob");
    Subscriber charlie("Charlie");

    auto conn1 = agency.news_alert.connect(
        [&alice](const std::string &h)
        { alice.receive(h); });

    auto conn2 = agency.news_alert.connect(
        [&bob](const std::string &h)
        { bob.receive(h); });

    agency.publish("Boost 1.91.0 released!");

    {
        // Charlie subscribes with scoped connection — auto-disconnects on scope exit
        boost::signals2::scoped_connection scoped_conn(
            agency.news_alert.connect(
                [&charlie](const std::string &h)
                { charlie.receive(h); }));

        agency.publish("Market opens higher");
    }

    // Charlie is disconnected now
    std::cout << '\n';
    agency.publish("Charlie should NOT receive this");

    // Disconnect Bob manually
    conn2.disconnect();
    std::cout << '\n';
    agency.publish("Only Alice should receive this");

    // Signal count
    std::cout << "\nConnected slots: " << agency.news_alert.num_slots() << '\n';

    return 0;
}
