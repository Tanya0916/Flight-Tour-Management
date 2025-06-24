#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <set>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

// Utility: Time conversion
string minutesToTime(int mins)
{
    int h = mins / 60;
    int m = mins % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << h << ":" << setw(2) << setfill('0') << m;
    return oss.str();
}
void clearConsole()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ---------------------- User Management ----------------------
class User
{
public:
    string username, password;
    User(string u, string p) : username(u), password(p) {}
    virtual ~User() {}
};

class Passenger : public User
{
public:
    string name;
    vector<int> bookings; // flightIDs
    Passenger() : User("test", "test"), name("test") {}
    Passenger(string u, string p, string n) : User(u, p), name(n) {}
};

class Admin : public User
{
public:
    Admin() : User("test", "test") {}
    Admin(string u, string p) : User(u, p) {}
};

// ---------------------- Crew Management ----------------------
struct CrewMember
{
    int id;
    string name;
    string role; // "Pilot", "Co-Pilot", "Attendant"
    set<int> assignedFlights;
    CrewMember() : id(0), name(""), role("") {}
    CrewMember(int i, string n, string r) : id(i), name(n), role(r) {}
};

// ---------------------- Flight and Booking ----------------------
struct Booking
{
    string passengerUsername;
    int seatNo;
    bool active;
    Booking(string u, int s) : passengerUsername(u), seatNo(s), active(true) {}
};

struct Flight
{
    int flightID;
    string source;
    string destination;
    int departureTime; // minutes from midnight
    int arrivalTime;
    int seatsTotal;
    int seatsAvailable;
    vector<bool> seatMap;
    vector<Booking> bookings;
    queue<string> waitlist;
    double basePrice;
    vector<int> crewAssigned; // crew IDs

    Flight(int id, string src, string dest, int dep, int arr, int seats, double price)
        : flightID(id), source(src), destination(dest), departureTime(dep),
          arrivalTime(arr), seatsTotal(seats), seatsAvailable(seats),
          seatMap(seats, false), basePrice(price) {}
};
double dynamicPrice(const Flight &f)
{
    double occupancy = 1.0 - (double)f.seatsAvailable / f.seatsTotal;
    return f.basePrice * (1 + occupancy * 0.5); // up to 50% more at full occupancy
}
// ---------------------- Airport Graph for Route Optimization ----------------------
class AirportGraph
{
public:
    unordered_map<string, vector<pair<string, int>>> adj;
    unordered_map<string, vector<pair<string, double>>> adj_price;

    void addFlight(const Flight &f)
    {
        int duration = f.arrivalTime - f.departureTime;
        adj[f.source].push_back(make_pair(f.destination, duration));
        adj_price[f.source].push_back(make_pair(f.destination, dynamicPrice(f)));
    }

    void removeFlight(const Flight &f)
    {
        vector<pair<string, int>> &edges = adj[f.source];
        edges.erase(remove_if(edges.begin(), edges.end(),
                              [&](pair<string, int> &p)
                              { return p.first == f.destination; }),
                    edges.end());
    }

    // Dijkstra's algorithm for shortest time
    int shortestRoute(string src, string dest, vector<string> &path)
    {
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;
        for (unordered_map<string, vector<pair<string, int>>>::iterator it = adj.begin(); it != adj.end(); ++it)
        {
            dist[it->first] = numeric_limits<int>::max();
        }
        dist[src] = 0;

        typedef pair<int, string> PQElem;
        priority_queue<PQElem, vector<PQElem>, greater<PQElem>> pq;
        pq.push(make_pair(0, src));

        while (!pq.empty())
        {
            PQElem top = pq.top();
            pq.pop();
            int curDist = top.first;
            string u = top.second;
            if (u == dest)
                break;
            vector<pair<string, int>> &neighbors = adj[u];
            for (size_t i = 0; i < neighbors.size(); ++i)
            {
                string v = neighbors[i].first;
                int weight = neighbors[i].second;
                if (curDist + weight < dist[v])
                {
                    dist[v] = curDist + weight;
                    prev[v] = u;
                    pq.push(make_pair(dist[v], v));
                }
            }
        }
        if (dist[dest] == numeric_limits<int>::max())
            return -1;

        // Build path
        path.clear();
        string cur = dest;
        while (cur != src)
        {
            path.push_back(cur);
            cur = prev[cur];
        }
        path.push_back(src);
        reverse(path.begin(), path.end());
        return dist[dest];
    }

    int cheapestRoute(string src, string dest, vector<string> &path)
    {
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;

        for (const auto &entry : adj_price)
        {
            dist[entry.first] = numeric_limits<int>::max();
        }
        dist[src] = 0;

        typedef pair<int, string> PQElem;
        priority_queue<PQElem, vector<PQElem>, greater<PQElem>> pq;
        pq.push(make_pair(0, src));

        while (!pq.empty())
        {
            PQElem top = pq.top();
            pq.pop();

            int curCost = top.first;
            string u = top.second;

            if (u == dest)
                break;

            for (const auto &neighbor : adj_price[u])
            {
                string v = neighbor.first;
                int cost = neighbor.second;

                if (curCost + cost < dist[v])
                {
                    dist[v] = curCost + cost;
                    prev[v] = u;
                    pq.push(make_pair(dist[v], v));
                }
            }
        }

        if (dist[dest] == numeric_limits<int>::max())
            return -1;

        // Build path
        path.clear();
        string cur = dest;
        while (cur != src)
        {
            path.push_back(cur);
            cur = prev[cur];
        }
        path.push_back(src);
        reverse(path.begin(), path.end());

        return dist[dest];
    }
};

// ---------------------- Main System Class ----------------------
class AirlinesSystem
{
private:
    vector<Flight> flights;
    unordered_map<string, Passenger> passengers;
    unordered_map<string, Admin> admins;
    unordered_map<int, CrewMember> crew;
    AirportGraph airportGraph;
    int nextFlightID = 1000;
    int nextCrewID = 1;

    // --- Helper Functions ---
    int binarySearchFlight(int flightID)
    {
        int left = 0, right = flights.size() - 1;
        while (left <= right)
        {
            int mid = left + (right - left) / 2;
            if (flights[mid].flightID == flightID)
                return mid;
            if (flights[mid].flightID < flightID)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return -1;
    }

    int assignSeat(Flight &flight)
    {
        for (int i = 0; i < flight.seatsTotal; ++i)
        {
            if (!flight.seatMap[i])
            {
                flight.seatMap[i] = true;
                flight.seatsAvailable--;
                return i + 1;
            }
        }
        return -1;
    }

    bool isCrewAvailable(int crewID, int dep, int arr)
    {
        // Get the set of flights already assigned to this crew member
        set<int> &assigned = crew[crewID].assignedFlights;
        // Check for time conflicts with each assigned flight
        for (set<int>::iterator it = assigned.begin(); it != assigned.end(); ++it)
        {
            int fid = *it;
            int idx = binarySearchFlight(fid);
            if (idx == -1)
                continue;
            Flight &f = flights[idx];
            // If times overlap, crew is not available for this flight
            if (!(arr <= f.departureTime || dep >= f.arrivalTime))
            {
                return false;
            }
        }
        // No conflicts found, crew is available
        return true;
    }

    // Backtracking for crew assignment to a flight
    bool assignCrewToFlight(int flightIdx, vector<int> &pilotIDs, vector<int> &attendantIDs)
    {
        Flight &f = flights[flightIdx];

        // Count already assigned pilots and attendants for this flight
        int pilotCount = 0, attendantCount = 0;
        for (int cid : f.crewAssigned)
        {
            if (crew[cid].role == "Pilot")
                pilotCount++;
            else if (crew[cid].role == "Attendant")
                attendantCount++;
        }
        // If already enough crew assigned, skip assignment
        if (pilotCount >= 2 && attendantCount >= 2)
        {
            cout << "Flight " << f.flightID << ": Already has 2 pilots and 2 attendants assigned. Skipping assignment.\n";
            return false;
        }

        // Find available pilots and attendants
        vector<int> assignedPilots, assignedAttendants;
        for (size_t i = 0; i < pilotIDs.size() && assignedPilots.size() < 2 - pilotCount; ++i)
        {
            int pid = pilotIDs[i];
            if (isCrewAvailable(pid, f.departureTime, f.arrivalTime))
                assignedPilots.push_back(pid);
        }
        for (size_t i = 0; i < attendantIDs.size() && assignedAttendants.size() < 2 - attendantCount; ++i)
        {
            int aid = attendantIDs[i];
            if (isCrewAvailable(aid, f.departureTime, f.arrivalTime))
                assignedAttendants.push_back(aid);
        }

        int totalPilots = pilotCount + assignedPilots.size();
        int totalAttendants = attendantCount + assignedAttendants.size();

        if (totalPilots == 2 && totalAttendants == 2)
        {
            // Assign crew
            for (int pid : assignedPilots)
            {
                f.crewAssigned.push_back(pid);
                crew[pid].assignedFlights.insert(flightID);
            }
            for (int aid : assignedAttendants)
            {
                f.crewAssigned.push_back(aid);
                crew[aid].assignedFlights.insert(flightID);
            }
            // cout << "Flight " << f.flightID << ": Crew assigned";
            // cout << "  Pilots: ";
            // for (int cid : f.crewAssigned) if (crew[cid].role == "Pilot") cout << cid << " ";
            // cout << "\n  Attendants: ";
            // for (int cid : f.crewAssigned) if (crew[cid].role == "Attendant") cout << cid << " ";
            // cout << "\n";
            return true;
        }
        else
        {
            int neededPilots = 2 - totalPilots;
            int neededAttendants = 2 - totalAttendants;
            cout << "Flight " << f.flightID << ": Could not assign required crew. Needed ";
            return false;
        }
    }

public:
    AirlinesSystem()
    {
        // Add a default admin
        admins["admin"] = Admin("admin", "admin123");
        // Add some crew
        crew[nextCrewID] = CrewMember(nextCrewID, "John Pilot", "Pilot");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Jane CoPilot", "Pilot");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Alice Attendant", "Attendant");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Bob Attendant", "Attendant");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Divyansh", "Pilot");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Somu", "Pilot");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Aditya", "Attendant");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Arman", "Attendant");
        nextCrewID++;
        crew[nextCrewID] = CrewMember(nextCrewID, "Tanya", "Attendant");
        nextCrewID++;
    }

    // --- User Authentication ---
    bool registerPassenger(string username, string password, string name)
    {
        if (passengers.count(username) || admins.count(username))
            return false;
        passengers[username] = Passenger(username, password, name);
        return true;
    }

    Passenger *loginPassenger(string username, string password)
    {
        if (passengers.count(username) && passengers[username].password == password)
            return &passengers[username];
        return NULL;
    }

    Admin *loginAdmin(string username, string password)
    {
        if (admins.count(username) && admins[username].password == password)
            return &admins[username];
        return NULL;
    }

    // --- Flight Management (Admin) ---
    void addFlight(string src, string dest, int dep, int arr, int seats, double price)
    {
        Flight f(nextFlightID++, src, dest, dep, arr, seats, price);
        flights.push_back(f);
        sort(flights.begin(), flights.end(), [](const Flight &a, const Flight &b)
             { return a.flightID < b.flightID; });
        airportGraph.addFlight(flights.back());
        cout << "Flight added: ID " << flights.back().flightID << endl;
    }

    void removeFlight(int flightID)
    {
        int idx = binarySearchFlight(flightID);
        if (idx == -1)
        {
            cout << "Flight not found.\n";
            return;
        }
        airportGraph.removeFlight(flights[idx]);
        flights.erase(flights.begin() + idx);
        cout << "Flight removed.\n";
    }

    void updateFlight(int flightID, int dep, int arr, int seats, double price)
    {
        int idx = binarySearchFlight(flightID);
        if (idx == -1)
        {
            cout << "Flight not found.\n";
            return;
        }
        flights[idx].departureTime = dep;
        flights[idx].arrivalTime = arr;
        flights[idx].seatsTotal = seats;
        flights[idx].seatsAvailable = seats;
        flights[idx].basePrice = price;
        flights[idx].seatMap = vector<bool>(seats, false);
        cout << "Flight updated.\n";
    }

    void listFlights()
    {

        cout << "Available Flights:\n";
        for (size_t i = 0; i < flights.size(); ++i)
        {
            const Flight &f = flights[i];
            cout << "ID: " << f.flightID << " | " << f.source << "->" << f.destination
                 << " | Dep: " << minutesToTime(f.departureTime)
                 << " | Arr: " << minutesToTime(f.arrivalTime)
                 << " | Seats: " << f.seatsAvailable << "/" << f.seatsTotal
                 << " | Price: " << dynamicPrice(f) << endl;
        }
    }

    // --- Crew Management (Admin) ---
    void addCrew(string name, string role)
    {
        crew[nextCrewID] = CrewMember(nextCrewID, name, role);
        cout << "Crew added: ID " << nextCrewID << ", " << name << ", " << role << endl;
        nextCrewID++;
    }

    void assignCrewToAllFlights()
    {
        vector<int> pilotIDs, attendantIDs;
        for (unordered_map<int, CrewMember>::iterator it = crew.begin(); it != crew.end(); ++it)
        {
            int id = it->first;
            CrewMember &c = it->second;
            if (c.role == "Pilot")
                pilotIDs.push_back(id);
            else if (c.role == "Attendant")
                attendantIDs.push_back(id);
        }
        for (size_t i = 0; i < flights.size(); ++i)
        {
            if (assignCrewToFlight(i, pilotIDs, attendantIDs))
                cout << "Crew assigned to flight " << flights[i].flightID << endl;
            else
                cout << "Could not assign crew to flight " << flights[i].flightID << endl;
        }
    }

    void listCrew()
    {
        for (unordered_map<int, CrewMember>::iterator it = crew.begin(); it != crew.end(); ++it)
        {
            int id = it->first;
            CrewMember &c = it->second;
            cout << "ID: " << id << ", Name: " << c.name << ", Role: " << c.role << endl;
        }
    }
    pair<int, int> minCrewRequired(vector<Flight> &flights)
    {
        sort(flights.begin(), flights.end(), [](const Flight &a, const Flight &b)
             { return a.departureTime < b.departureTime; });

        priority_queue<int, vector<int>, greater<int>> pilotAvailable;
        priority_queue<int, vector<int>, greater<int>> attendantAvailable;

        int totalPilots = 0;
        int totalAttendants = 0;

        for (const Flight &f : flights)
        {
            int pilotsAssigned = 0;
            int attendantsAssigned = 0;

            while (!pilotAvailable.empty() && pilotAvailable.top() <= f.departureTime && pilotsAssigned < 2)
            {
                pilotAvailable.pop();
                pilotsAssigned++;
            }
            totalPilots += (2 - pilotsAssigned);

            while (!attendantAvailable.empty() && attendantAvailable.top() <= f.departureTime && attendantsAssigned < 2)
            {
                attendantAvailable.pop();
                attendantsAssigned++;
            }
            totalAttendants += (2 - attendantsAssigned);

            for (int i = 0; i < 2; ++i)
            {
                pilotAvailable.push(f.arrivalTime);
                attendantAvailable.push(f.arrivalTime);
            }
        }

        return {totalPilots, totalAttendants};
    }
    void checkCrewVacancy()
    {
        int currentPilotsAvailable = 0;
        int currentAttendantsAvailable = 0;

        for (auto it = crew.begin(); it != crew.end(); ++it)
        {
            CrewMember &c = it->second;
            if (c.role == "Pilot")
                currentPilotsAvailable++;
            else if (c.role == "Attendant")
                currentAttendantsAvailable++;
        }

        pair<int, int> required = minCrewRequired(flights);

        int extraPilots = max(0, required.first - currentPilotsAvailable);
        int extraAttendants = max(0, required.second - currentAttendantsAvailable);

        cout << "Add " << extraPilots << " more pilots" << endl;
        cout << "Add " << extraAttendants << " more attendants" << endl;
    }

    void searchFlights(string src, string dest)
    {
        if (src.empty() || dest.empty())
        {
            cout << "Error: Source or destination cannot be empty.\n";
            return;
        }

        if (flights.empty())
        {
            cout << "Error: No flights available.\n";
            return;
        }

        cout << "Flights from " << src << " to " << dest << ":\n";

        bool found = false;
        for (size_t i = 0; i < flights.size(); ++i)
        {
            const Flight &f = flights[i];

            if (f.source == src && f.destination == dest)
            {
                found = true;
                cout << "ID: " << f.flightID << " | Dep: " << minutesToTime(f.departureTime)
                     << " | Arr: " << minutesToTime(f.arrivalTime)
                     << " | Seats: " << f.seatsAvailable << "/" << f.seatsTotal
                     << " | Price: " << dynamicPrice(f) << endl;
            }
        }

        if (!found)
        {
            cout << "Error: No flights found from " << src << " to " << dest << ".\n";
        }
    }

    void searchFlightsByTime(int earliestDep, int latestDep)
    {
        if (earliestDep > latestDep)
        {
            cout << "Error: Earliest departure time cannot be later than latest departure time.\n";
            return;
        }

        if (flights.empty())
        {
            cout << "No flights available.\n";
            return;
        }

        cout << "Flights departing between " << minutesToTime(earliestDep) << " and " << minutesToTime(latestDep) << ":\n";

        bool found = false;
        for (size_t i = 0; i < flights.size(); ++i)
        {
            const Flight &f = flights[i];

            if (f.departureTime < 0 || f.departureTime > 1440)
            {
                cout << "Error: Invalid departure time for flight ID " << f.flightID << ". Skipping this flight.\n";
                continue;
            }

            if (f.departureTime >= earliestDep && f.departureTime <= latestDep)
            {
                found = true;
                cout << "ID: " << f.flightID << " | " << f.source << "->" << f.destination
                     << " | Dep: " << minutesToTime(f.departureTime)
                     << " | Arr: " << minutesToTime(f.arrivalTime)
                     << " | Seats: " << f.seatsAvailable << "/" << f.seatsTotal
                     << " | Price: " << dynamicPrice(f) << endl;
            }
        }

        if (!found)
        {
            cout << "No flights found within the specified time range.\n";
        }
    }

    void bookFlight(string username, int flightID)
    {
        int idx = binarySearchFlight(flightID);
        if (idx == -1)
        {
            cout << "Flight not found.\n";
            return;
        }
        Flight &f = flights[idx];
        if (f.seatsAvailable > 0)
        {
            int seatNo = assignSeat(f);
            f.bookings.push_back(Booking(username, seatNo));
            passengers[username].bookings.push_back(flightID);
            cout << "Seat booked! Flight " << flightID << ", Seat #" << seatNo
                 << ", Price: " << dynamicPrice(f) << endl;
        }
        else
        {
            cout << "No seats available. Added to waitlist.\n";
            f.waitlist.push(username);
        }
    }

    void cancelBooking(string username, int flightID)
    {
        int idx = binarySearchFlight(flightID);
        if (idx == -1)
        {
            cout << "Flight not found.\n";
            return;
        }
        Flight &f = flights[idx];
        bool found = false;
        for (size_t i = 0; i < f.bookings.size(); ++i)
        {
            Booking &b = f.bookings[i];
            if (b.passengerUsername == username && b.active)
            {
                b.active = false;
                f.seatMap[b.seatNo - 1] = false;
                f.seatsAvailable++;
                found = true;
                break;
            }
        }
        if (found)
        {
            cout << "Booking cancelled.\n";
            // Assign seat to waitlist if any
            if (!f.waitlist.empty())
            {
                string nextUser = f.waitlist.front();
                f.waitlist.pop();
                int seatNo = assignSeat(f);
                f.bookings.push_back(Booking(nextUser, seatNo));
                passengers[nextUser].bookings.push_back(flightID);
                cout << "Waitlisted passenger " << nextUser << " booked on this flight, seat #" << seatNo << endl;
            }
        }
        else
        {
            cout << "No active booking found for this user on this flight.\n";
        }
    }

    void listPassengerBookings(string username)
    {
        if (!passengers.count(username))
        {
            cout << "Passenger not found.\n";
            return;
        }

        cout << "Bookings for " << username << ":\n";

        bool foundBooking = false;

        for (Flight &f : flights)
        {
            for (Booking &b : f.bookings)
            {
                if (b.passengerUsername == username && b.active)
                {
                    foundBooking = true;
                    cout << "Flight " << f.flightID << " | " << f.source << "->" << f.destination
                         << " | Dep: " << minutesToTime(f.departureTime)
                         << " | Seat #" << b.seatNo << endl;
                }
            }
        }

        if (!foundBooking)
        {
            cout << "No bookings available.\n";
        }
    }

    // --- Route Optimization ---
    void findShortestRoute(string src, string dest)
    {
        vector<string> path;
        int mins = airportGraph.shortestRoute(src, dest, path);
        if (mins == -1)
            cout << "No route found.\n";
        else
        {
            cout << "Shortest route (" << mins << " mins): ";
            for (size_t i = 0; i < path.size(); ++i)
            {
                cout << path[i];
                if (i < path.size() - 1)
                    cout << " -> ";
            }
            cout << endl;
        }
    }
    void findCheapestRoute(string src, string dest)
    {
        vector<string> path;
        int mins = airportGraph.cheapestRoute(src, dest, path);
        if (mins == -1)
            cout << "No route found.\n";
        else
        {
            cout << "Cheapest route (" << mins << " INR): ";
            for (size_t i = 0; i < path.size(); ++i)
            {
                cout << path[i];
                if (i < path.size() - 1)
                    cout << " -> ";
            }
            cout << endl;
        }
    }

    // --- Reporting ---
    void flightOccupancyReport()
    {
        cout << "Flight Occupancy Report:\n";
        for (size_t i = 0; i < flights.size(); ++i)
        {
            const Flight &f = flights[i];
            double occ = 100.0 * (f.seatsTotal - f.seatsAvailable) / f.seatsTotal;
            cout << "Flight " << f.flightID << ": " << occ << "% full\n";
        }
    }

    void waitlistReport()
    {
        cout << "Waitlist Report:\n";
        for (size_t i = 0; i < flights.size(); ++i)
        {
            const Flight &f = flights[i];
            cout << "Flight " << f.flightID << ": " << f.waitlist.size() << " on waitlist\n";
        }
    }
    void printAllDuties();
};
void AirlinesSystem::printAllDuties()
{
    cout << "\n----- Crew Duties -----\n";
    for (const auto &pair : crew)
    {
        const CrewMember &c = pair.second;
        cout << "Crew ID: " << c.id << " | Role: " << c.role << "\n";
        if (c.assignedFlights.empty())
        {
            cout << "  No flights assigned.\n";
        }
        else
        {
            for (int fid : c.assignedFlights)
            {
                auto it = find_if(flights.begin(), flights.end(), [&](const Flight &f)
                                  { return f.flightID == fid; });
                if (it != flights.end())
                {
                    cout << "  Flight ID: " << it->flightID
                         << " | Departure: " << it->departureTime
                         << " | Arrival: " << it->arrivalTime << "\n";
                }
                else
                {
                    cout << "  Flight ID: " << fid << " (details not found)\n";
                }
            }
        }
        cout << "-------------------------\n";
    }
}

// ---------------------- Main Menu ----------------------
void adminMenu(AirlinesSystem &sys)
{
    while (true)
    {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Flight\n2. Remove Flight\n3. Update Flight\n4. List Flights\n";
        cout << "5. Add Crew\n6. Assign Crew\n7. List Crew\n";
        cout << "8. Flight Occupancy Report\n9. Waitlist Report\n";
        cout << "10.View All Duties\n11.Check Crew Vaccancy \n 0. Logout\n";
        int ch;
        cin >> ch;
        if (ch == 0)
            break;
        if (ch == 1)
        {
            int a;
            cout << "Tell Me Captain ! 😉 How many Flights are we Adding to The Runway? :  ";
            cin >> a;
            int n = 1;
            while (a--)
            {
                cout << "Enter Details of Flight No. " << n << endl;
                n++;
                string src, dest;
                int dep, arr, seats;
                double price;
                cout << "Source: ";
                cin >> src;
                cout << "Destination: ";
                cin >> dest;
                cout << "Departure (min from midnight): ";
                cin >> dep;
                cout << "Arrival (min from midnight): ";
                cin >> arr;
                cout << "Seats: ";
                cin >> seats;
                cout << "Base Price: ";
                cin >> price;
                sys.addFlight(src, dest, dep, arr, seats, price);
            }
        }
        else if (ch == 2)
        {
            int a;
            cout << "Captain, how many flights are we clearing for deletion?";
            cin >> a;
            int n = 1;
            while (a--)
            {
                int fid;
                cout << "Flight ID: ";
                cin >> fid;
                sys.removeFlight(fid);
                cout << "Off the list, out of sight — say goodbye to this flight!" << endl;
            }
        }
        else if (ch == 3)
        {
            int fid, dep, arr, seats;
            double price;
            cout << "Flight ID: ";
            cin >> fid;
            cout << "New Departure (min): ";
            cin >> dep;
            cout << "New Arrival (min): ";
            cin >> arr;
            cout << "New Seats: ";
            cin >> seats;
            cout << "New Base Price: ";
            cin >> price;
            sys.updateFlight(fid, dep, arr, seats, price);
        }
        else if (ch == 4)
        {
            sys.listFlights();
        }
        else if (ch == 5)
        {
            int a;
            cout << "How many new faces are joining the flight crew?" << endl;
            cin >> a;
            while (a--)
            {
                string name, role;
                cout << "Crew Name: ";
                cin >> name;
                cout << "Role (Pilot/Attendant): ";
                cin >> role;
                sys.addCrew(name, role);
                cout << name << " is now taking on the role of " << role << " for FlightEase. Let us  all welcome " << name << "to the FlightEase family!" << endl;
            }
        }
        else if (ch == 6)
        {
            sys.assignCrewToAllFlights();
        }
        else if (ch == 7)
        {
            sys.listCrew();
        }
        else if (ch == 8)
        {
            sys.flightOccupancyReport();
        }
        else if (ch == 9)
        {
            sys.waitlistReport();
        }
        else if (ch == 10)
        {
            sys.printAllDuties();
        }
        else if (ch == 11)
        {
            sys.checkCrewVacancy();
        }
    }
}

void passengerMenu(AirlinesSystem &sys, string username)
{
    while (true)
    {
        cout << "\n--- Passenger Menu ---\n";
        cout << "1. Available Flights\n2. Search Flights by Route\n3. Search Flights by Time\n4. Book Flight\n";
        cout << "5. Cancel Booking\n6. My Bookings\n7. Find Shortest Route\n8.Find Cheapest Route\n0. Logout\n";
        int ch;
        cin >> ch;
        if (ch == 0)
            break;
        if (ch == 1)
        {
            sys.listFlights();
        }
        if (ch == 2)
        {
            string src, dest;
            cout << "Source: ";
            cin >> src;
            cout << "Destination: ";
            cin >> dest;
            sys.searchFlights(src, dest);
        }
        else if (ch == 3)
        {
            int dep1, dep2;
            cout << "Earliest Departure (min): ";
            cin >> dep1;
            cout << "Latest Departure (min): ";
            cin >> dep2;
            sys.searchFlightsByTime(dep1, dep2);
        }
        else if (ch == 4)
        {
            int fid;
            cout << "Flight ID to book: ";
            cin >> fid;
            sys.bookFlight(username, fid);
        }
        else if (ch == 5)
        {
            int fid;
            cout << "Flight ID to cancel: ";
            cin >> fid;
            sys.cancelBooking(username, fid);
        }
        else if (ch == 6)
        {
            sys.listPassengerBookings(username);
        }
        else if (ch == 7)
        {
            string src, dest;
            cout << "Source: ";
            cin >> src;
            cout << "Destination: ";
            cin >> dest;
            sys.findShortestRoute(src, dest);
        }
        else if (ch == 8)
        {
            string src, dest;
            cout << "Source: ";
            cin >> src;
            cout << "Destination: ";
            cin >> dest;
            sys.findCheapestRoute(src, dest);
        }
    }
}

int main()
{
    AirlinesSystem sys;
    // Preload some flights
    sys.addFlight("DEL", "MUM", 480, 660, 3, 5000);
    sys.addFlight("MUM", "BLR", 700, 900, 2, 4000);
    sys.addFlight("DEL", "BLR", 500, 900, 1, 7000);
    clearConsole();
    cout << endl;
    cout << "                 |  ____|| |     |_   _/ ____| |  | |__   __| |  ____|   /\\    / ____|  ____|" << endl;
    cout << "                 | |__   | |       | || |  __| |__| |  | |    | |__     /  \\  | (___ | |__   " << endl;
    cout << "                 |  __|  | |       | || | |_ |  __  |  | |    |  __|   / /\\ \\  \\___ \\|  __|  " << endl;
    cout << "                 | |     | |____  _| || |__| | |  | |  | |    | |____ / ____ \\ ____) | |____" << endl;
    cout << "                 |_|     |______||_____\\_____|_|  |_|  |_|    |______/_/    \\_\\_____/|______|" << endl;

    while (true)
    {
        cout << "\n--- Airlines Management System ---\n";
        cout << "1. Admin Login\n2. Passenger Login\n3. Passenger Registration\n0. Exit\n";
        int ch;
        cin >> ch;
        if (ch == 0)
            break;
        if (ch == 1)
        {
            string u, p;
            cout << "Admin Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            Admin *admin = sys.loginAdmin(u, p);
            if (admin)
                adminMenu(sys);
            else
                cout << "Invalid admin credentials.\n";
        }
        else if (ch == 2)
        {

            string u, p;
            cout << "Passenger Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            Passenger *pass = sys.loginPassenger(u, p);
            if (pass)
                passengerMenu(sys, u);
            else
                cout << "Invalid passenger credentials.\n";
        }
        else if (ch == 3)
        {

            string u, p, n;
            cout << "Choose Username: ";
            cin >> u;
            cout << "Choose Password: ";
            cin >> p;
            cin.ignore();
            cout << "Your Name: ";
            getline(cin, n);
            if (sys.registerPassenger(u, p, n))
                cout << "Registration successful. Please login.\n";
            else
                cout << "Username already exists.\n";
        }
    }
    cout << "Thank you for using Airlines Management System!\n";
    return 0;
}