#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// AquaSave: a console-based water conservation tracker.
// Data is stored in water_records.csv in the same folder as the program.

struct WaterRecord {
    string date;
    string activity;
    double usedLitres;
    double wastedLitres;
};

const string DATA_FILE = "water_records.csv";

string cleanField(string value) {
    // Keep the CSV format simple by replacing commas and newlines.
    replace(value.begin(), value.end(), ',', ';');
    replace(value.begin(), value.end(), '\n', ' ');
    replace(value.begin(), value.end(), '\r', ' ');
    return value;
}

bool isValidDate(const string& date) {
    // Simple format check: YYYY-MM-DD (not a full calendar-date validator).
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    return true;
}

double readNonNegative(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Please enter a valid number (0 or greater).\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readInt(const string& prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minValue && value <= maxValue) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Please enter a whole number from " << minValue
             << " to " << maxValue << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

vector<WaterRecord> loadRecords() {
    vector<WaterRecord> records;
    ifstream file(DATA_FILE);
    if (!file) return records;

    string line;
    while (getline(file, line)) {
        if (line.empty() || line.rfind("date,", 0) == 0) continue;
        stringstream ss(line);
        WaterRecord r;
        string used, wasted;
        if (getline(ss, r.date, ',') &&
            getline(ss, r.activity, ',') &&
            getline(ss, used, ',') &&
            getline(ss, wasted, ',')) {
            try {
                r.usedLitres = stod(used);
                r.wastedLitres = stod(wasted);
                if (r.usedLitres >= 0 && r.wastedLitres >= 0 &&
                    r.wastedLitres <= r.usedLitres) {
                    records.push_back(r);
                }
            } catch (...) {
                // Ignore malformed rows instead of crashing.
            }
        }
    }
    return records;
}

bool saveRecord(const WaterRecord& r) {
    bool needsHeader = false;
    ifstream check(DATA_FILE);
    needsHeader = !check.good() || check.peek() == ifstream::traits_type::eof();
    check.close();

    ofstream file(DATA_FILE, ios::app);
    if (!file) {
        cout << "Error: could not open " << DATA_FILE << " for writing.\n";
        return false;
    }
    if (needsHeader) file << "date,activity,used_litres,wasted_litres\n";
    file << cleanField(r.date) << ','
         << cleanField(r.activity) << ','
         << fixed << setprecision(2) << r.usedLitres << ','
         << r.wastedLitres << '\n';
    return true;
}

void addRecord() {
    WaterRecord r;
    cout << "\n--- Add Water Usage Record ---\n";
    do {
        cout << "Date (YYYY-MM-DD): ";
        getline(cin, r.date);
        if (!isValidDate(r.date))
            cout << "Use the format YYYY-MM-DD, for example 2026-10-16.\n";
    } while (!isValidDate(r.date));

    cout << "Activity (e.g., bathing, laundry, gardening): ";
    getline(cin, r.activity);
    if (r.activity.empty()) r.activity = "Unspecified activity";

    r.usedLitres = readNonNegative("Total water used (litres): ");
    while (true) {
        r.wastedLitres = readNonNegative("Estimated water wasted (litres): ");
        if (r.wastedLitres <= r.usedLitres) break;
        cout << "Wasted water cannot exceed total water used. Try again.\n";
    }

    if (saveRecord(r))
        cout << "Record saved successfully.\n";
}

void viewRecords(const vector<WaterRecord>& records) {
    cout << "\n--- Water Usage Records ---\n";
    if (records.empty()) {
        cout << "No records yet. Add a record first.\n";
        return;
    }

    cout << left << setw(14) << "Date"
         << setw(24) << "Activity"
         << right << setw(14) << "Used (L)"
         << setw(14) << "Wasted (L)" << '\n';
    cout << string(66, '-') << '\n';
    cout << fixed << setprecision(2);
    for (const auto& r : records) {
        cout << left << setw(14) << r.date
             << setw(24) << r.activity.substr(0, 22)
             << right << setw(14) << r.usedLitres
             << setw(14) << r.wastedLitres << '\n';
    }
}

void showReport(const vector<WaterRecord>& records, double dailyTarget) {
    cout << "\n--- AquaSave Summary Report ---\n";
    if (records.empty()) {
        cout << "No data available. Add records to generate a report.\n";
        return;
    }

    double totalUsed = 0, totalWasted = 0;
    for (const auto& r : records) {
        totalUsed += r.usedLitres;
        totalWasted += r.wastedLitres;
    }
    double average = totalUsed / records.size();
    double wastePercent = totalUsed > 0 ? (totalWasted / totalUsed) * 100.0 : 0.0;
    double targetTotal = dailyTarget * records.size();

    cout << fixed << setprecision(2);
    cout << "Number of records: " << records.size() << '\n';
    cout << "Total water used: " << totalUsed << " litres\n";
    cout << "Total estimated water wasted: " << totalWasted << " litres\n";
    cout << "Average water used per record: " << average << " litres\n";
    cout << "Estimated wastage rate: " << wastePercent << "%\n";
    cout << "Target per record: " << dailyTarget << " litres\n";
    cout << "Combined target for these records: " << targetTotal << " litres\n";

    if (totalUsed > targetTotal) {
        cout << "Status: Usage is above the combined target by "
             << totalUsed - targetTotal << " litres.\n";
    } else {
        cout << "Status: Usage is within the combined target, with "
             << targetTotal - totalUsed << " litres below target.\n";
    }

    cout << "\nRecommendations:\n";
    if (wastePercent >= 10.0) {
        cout << "- Estimated wastage is 10% or more. Check taps, pipes, and storage tanks for leaks.\n";
    } else {
        cout << "- Keep checking for leaks and record wastage accurately.\n";
    }
    if (average > dailyTarget) {
        cout << "- Average usage is above your target. Review bathing, laundry, cleaning, and gardening habits.\n";
    } else {
        cout << "- Average usage is at or below your target. Continue water-saving habits.\n";
    }
    cout << "- Use a bucket where practical, repair leaking taps, and reuse suitable greywater for plants.\n";
    cout << "- These are estimates based on entered records, not measurements from a physical sensor.\n";
}

void showTips() {
    cout << "\n--- Water Conservation Tips ---\n"
         << "1. Repair leaking taps and pipes promptly.\n"
         << "2. Turn off taps while brushing or scrubbing dishes.\n"
         << "3. Prefer a bucket over a long shower when practical.\n"
         << "4. Run washing machines with suitable full loads.\n"
         << "5. Water plants early or late to reduce evaporation.\n"
         << "6. Reuse suitable leftover water for cleaning or plants; avoid unsafe reuse.\n"
         << "7. Consider rainwater harvesting where feasible and permitted.\n";
}

int main() {
    double dailyTarget = 150.0;
    cout << "=====================================\n";
    cout << "       AquaSave Water Tracker\n";
    cout << " SDG 6: Clean Water and Sanitation\n";
    cout << "=====================================\n";

    dailyTarget = readNonNegative("Set your water-use target per record/day (litres, default 150): ");
    if (dailyTarget == 0) {
        dailyTarget = 150.0;
        cout << "Using default target: 150 litres.\n";
    }

    int choice;
    do {
        cout << "\n========== MAIN MENU ==========\n"
             << "1. Add water usage record\n"
             << "2. View saved records\n"
             << "3. View summary report\n"
             << "4. Water conservation tips\n"
             << "5. Exit\n";
        choice = readInt("Choose an option (1-5): ", 1, 5);

        vector<WaterRecord> records;
        switch (choice) {
            case 1:
                addRecord();
                break;
            case 2:
                records = loadRecords();
                viewRecords(records);
                break;
            case 3:
                records = loadRecords();
                showReport(records, dailyTarget);
                break;
            case 4:
                showTips();
                break;
            case 5:
                cout << "Thank you for using AquaSave. Every drop counts!\n";
                break;
        }
    } while (choice != 5);

    return 0;
}
