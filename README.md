# Airline Management System

This is a comprehensive Airline Management System developed in C++. It provides a command-line interface to manage flights, crew, passengers, and bookings. The system is designed with distinct roles for Administrators and Passengers, each with a specific set of functionalities. It also incorporates advanced features like dynamic pricing, route optimization using Dijkstra's algorithm, and crew management.

## 🚀 Features

The system is broadly divided into two user modules:

### 👨‍✈️ Admin Module

The admin has overarching control over the airline's operations. The key functionalities include:

  * **Flight Management:**

      * **Add Flight:** Add new flights to the system with details such as source, destination, departure and arrival times, seat capacity, and base price.
      * **Remove Flight:** Remove existing flights from the system using the flight ID.
      * **Update Flight:** Modify the details of an existing flight.
      * **List All Flights:** View a complete list of all available flights and their current status.

  * **Crew Management:**

      * **Add Crew:** Add new crew members (Pilots and Attendants) to the system.
      * **Assign Crew to Flights:** Automatically assign available crew members to flights. The system ensures that each flight has the required number of pilots and attendants and that there are no scheduling conflicts.
      * **List Crew:** View a list of all crew members and their roles.
      * **Check Crew Vacancy:** Determine the number of additional pilots and attendants required based on the current flight schedule.
      * **View All Duties:** Display the flight assignments for each crew member.

  * **Reporting:**

      * **Flight Occupancy Report:** Generate a report showing the percentage of occupied seats for each flight.
      * **Waitlist Report:** View the number of passengers on the waitlist for each flight.

### 🚶 Passenger Module

The passenger module allows users to interact with the airline for booking and travel purposes. The features are:

  * **User Authentication:**

      * **Registration:** New passengers can create an account.
      * **Login:** Registered passengers can log in to their accounts.

  * **Flight Interaction:**

      * **View Available Flights:** Browse a list of all flights.
      * **Search Flights by Route:** Find flights between a specific source and destination.
      * **Search Flights by Time:** Search for flights within a given departure time window.

  * **Booking and Cancellations:**

      * **Book Flight:** Book a seat on a flight. If no seats are available, the passenger is added to a waitlist.
      * **Cancel Booking:** Cancel a previously booked flight. If there is a waitlist, the first person on the list is automatically booked.
      * **My Bookings:** View a list of all personal flight bookings.

  * **Route Optimization:**

      * **Find Shortest Route:** Find the quickest travel route between two cities, which may involve connecting flights.
      * **Find Cheapest Route:** Find the most economical travel route between two cities based on dynamic flight prices.

## 🛠️ Technical Implementation & Data Structures

The system is implemented in C++ and utilizes various object-oriented programming principles and data structures:

  * **Classes and Objects:**

      * `User`, `Passenger`, and `Admin` classes for managing user data and authentication.
      * `Flight` struct to store all information related to a flight, including bookings and a waitlist.
      * `Booking` struct to manage individual booking details.
      * `CrewMember` struct to store information about the flight crew.
      * `AirlinesSystem` class as the main controller that encapsulates all the logic and data.

  * **Core Data Structures:**

      * `std::vector` to store lists of flights, bookings, and crew.
      * `std::unordered_map` for efficient lookups of passengers, admins, and crew members.
      * `std::queue` to manage the passenger waitlist for flights.
      * `std::set` to keep track of flights assigned to each crew member, preventing duplicate assignments.
      * `std::priority_queue` is used within the route optimization algorithms.

  * **Algorithms:**

      * **Dijkstra's Algorithm:** Implemented to find the shortest and cheapest routes in the `AirportGraph` class. The graph represents airports as nodes and flights as edges.
      * **Binary Search:** Used for efficiently finding flights by their ID.
      * **Backtracking:** A backtracking approach is used in the crew assignment logic to find a valid assignment of pilots and attendants to a flight.
      * **Sorting:** Flights are sorted by departure time to determine the minimum crew required.

  * **Dynamic Pricing:** The price of a flight ticket is dynamic and increases with the flight's occupancy, calculated by the `dynamicPrice` function.

## ⚙️ How to Compile and Run

1.  **Save the Code:** Save the provided C++ code as a `.cpp` file (e.g., `airline_system.cpp`).

2.  **Compile:** Use a C++ compiler like g++ to compile the code.

    ```bash
    g++ -o airline_system code.cpp
    ```

3.  **Run:** Execute the compiled program from your terminal.

    ```bash
    ./airline_system
    ```

## 📖 How to Use the System

Upon running the application, you will be greeted with the main menu.

### Main Menu

```
--- Airlines Management System ---
1. Admin Login
2. Passenger Login
3. Passenger Registration
0. Exit
```

  * **Admins:** To use the admin features, log in with the default credentials:
      * **Username:** `admin`
      * **Password:** `admin123`
  * **Passengers:** If you are a new passenger, you must register first by selecting option `3`. Then, you can log in with your created credentials.

### Example Workflows

#### As an Admin

1.  Log in as an admin.
2.  Select `1. Add Flight` to add a few flights.
3.  Select `5. Add Crew` to add pilots and attendants.
4.  Select `6. Assign Crew` to automatically schedule crew for the flights.
5.  Select `8. Flight Occupancy Report` to see the current booking status.

#### As a Passenger

1.  Register as a new passenger from the main menu.
2.  Log in with your new username and password.
3.  Select `1. Available Flights` to see all flight options.
4.  Select `2. Search Flights by Route` to find a specific trip.
5.  Select `4. Book Flight` and enter the ID of the flight you wish to book.
6.  Select `6. My Bookings` to confirm your booking.
7.  If you want to find the best route for a multi-city trip, use options `7. Find Shortest Route` or `8. Find Cheapest Route`.