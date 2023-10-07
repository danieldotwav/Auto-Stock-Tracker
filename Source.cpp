#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
using namespace std;

// Global Constants
const int MAX_RECORDS = 80; // (Actual Maximum Number of Records That Can Be Processed)
const int TEXT_WIDTH = 15; // Width for Car ID and Model 
const int NUM_WIDTH = 12; // Width for Quantity and Price
const int MIN_PRICE = 5000; // Minimum Vehicle Price
const int MIN_MODEL_LEN = 3; // Minimum Model Name Length
const int REQ_ID_LEN = 9; // Required Car ID length
const int ID_PART1_END = 2; // Length of first part of Car ID
const int ID_PART2_END = 7; // Length of the first and second part of Car ID
const int HEADER_WIDTH = 54; // Width for header

// Enumerated type for menu selection
enum Selection { VALID = 1, INVALID, SORT, QUIT };
enum Fields { ID = 1, MODEL, QUANTITY, PRICE, RETURN_TO_MAIN };

class Car {
private:
	string carID;
	string model;
	int quantity;
	double price;

public:
	Car() { setCar("N/a", "N/a", 0, 0); }
	Car(string n_carID, string n_model, int n_quantity, double n_price) { setCar(n_carID, n_model, n_quantity, n_price); }

	void setCar(string n_carID, string n_model, int n_quantity, double n_price);
	void setCarID(string n_carID) { setCar(n_carID, model, quantity, price); }
	void setModel(string n_model) { setCar(carID, n_model, quantity, price); }
	void setQuantity(int n_quantity) { setCar(carID, model, n_quantity, price); }
	void setPrice(double n_price) { setCar(carID, model, quantity, n_price); }

	string getCarID() const { return carID; }
	string getModel() const { return model; }
	int getQuantity() const { return quantity; }
	double getPrice() const { return price; }
	string toString() const;
};


class Inventory {
private:
	Car cars[MAX_RECORDS];
	Car* carPtrs[MAX_RECORDS];
	int numCars = 0;

public:
	Inventory() { ParseData(); }

	void setNumCars(int n_numCars) { numCars = n_numCars; }
	void ParseData();
	bool ValidateRecord(string carID, string model, int quantity, double price, string& errorMessage);
	bool ValidateCarID(string carID, string& errorMessage);
	bool ValidateModel(string model, string& errorMessage);
	bool ValidateQuantity(int quantity, string& errorMessage);
	bool ValidatePrice(double price, string& errorMessage);
	void SortBy(int field);

	void MakeStringUppercase(string& str);
	void PrintInventory();
};

// Function Prototypes
int GetMenuSelection();
void SortMenu(Inventory& inventory); // Wrapper function for sorting menu
int GetSortKey();
void PrintInvalidRecords();
void PurgeInputErrors(string errMess);

int main() {
	Inventory inventory;
	int selection;

	do {
		selection = GetMenuSelection();

		switch (selection) {
		case VALID:
			inventory.PrintInventory();
			break;
		case INVALID:
			PrintInvalidRecords();
			break;
		case SORT:
			SortMenu(inventory);
			break;
		case QUIT:
			cout << "Terminating Program\n";
			break;
		default:
			PurgeInputErrors("ERROR: Invalid input. Please enter a valid option.\n\n");
		}
	} while (selection != QUIT);

	return 0;
}

void Car::setCar(string n_carID, string n_model, int n_quantity, double n_price) {
	carID = n_carID;
	model = n_model;
	quantity = n_quantity;
	price = n_price;
}

string Car::toString() const {
	stringstream recordString;
	recordString << fixed << showpoint << setprecision(2) << left << setw(TEXT_WIDTH) << carID << setw(TEXT_WIDTH) << model
		<< setw(NUM_WIDTH) << right << quantity << setw(NUM_WIDTH) << price << left << endl;

	return recordString.str();
}

void Inventory::ParseData() {
	string fileName{ "Data.txt" };
	string errorFileName{ "ErrorFile.txt" };
	stringstream ssHeader;

	cout << fixed << showpoint << setprecision(2);
	ssHeader << fixed << showpoint << setprecision(2) << left
		<< setw(TEXT_WIDTH) << "Vehicle ID"
		<< setw(TEXT_WIDTH) << "Model"
		<< setw(NUM_WIDTH) << right << "Quantity"
		<< setw(NUM_WIDTH) << "Price" << left << "\n\n";

	ifstream Infile(fileName);
	if (!Infile.is_open()) {
		cout << "ERROR: Unable to open '" << fileName << "'. Terminating Program\n";
		exit(EXIT_FAILURE);
	}
	else if (Infile.peek() == EOF) {
		cout << "ERROR: '" << fileName << "' is empty. No data to process. Terminating Program\n";
		exit(EXIT_FAILURE);
	}

	ofstream Errfile(errorFileName);
	if (!Errfile) {
		cout << "ERROR: Unable to open '" << errorFileName << "'. Terminating Program\n";
		Infile.close();
		exit(EXIT_FAILURE);
	}

	string errorMessage{ "" };
	string carID, model;
	int quantity;
	double price;
	bool isValidRecord;
	int currentRecord{ 0 };
	int totalCars = 0;
	string line;

	while (getline(Infile, line) && currentRecord < MAX_RECORDS) {
		errorMessage = ""; // Reset error message before each line is read
		stringstream ss(line);
		ss >> carID >> model >> quantity >> price;

		isValidRecord = ValidateRecord(carID, model, quantity, price, errorMessage);

		if (isValidRecord) {
			MakeStringUppercase(carID);
			MakeStringUppercase(model);

			cars[totalCars].setCar(carID, model, quantity, price);
			carPtrs[totalCars] = &cars[totalCars];
			totalCars++;
		}
		else {
			Errfile << left << setw(TEXT_WIDTH) << carID << setw(TEXT_WIDTH) << model << setw(NUM_WIDTH) << right << quantity
				<< setw(NUM_WIDTH) << right << price << left << " " << errorMessage << "\n";
		}
		currentRecord++;
	}

	// Check if records processed exceeds the total number of records
	if (currentRecord == MAX_RECORDS && !Infile.eof()) {
		cout << "ERROR: The total number of records exceed the maximum limit of " << MAX_RECORDS
			<< ". Any records beyond the maximum " << MAX_RECORDS << " will be discarded.\n\n";
	}
	setNumCars(totalCars);
	Infile.close();
	Errfile.close();
}

// Calls individual validator functions and returns true if all validators return true
bool Inventory::ValidateRecord(const string carID, const string model, const int quantity, const double price, string& errorMessage) {
	bool validID, validModel, validQuantity, validPrice;

	validID = ValidateCarID(carID, errorMessage);
	validModel = ValidateModel(model, errorMessage);
	validQuantity = ValidateQuantity(quantity, errorMessage);
	validPrice = ValidatePrice(price, errorMessage);

	return (validID && validModel && validQuantity && validPrice);
}

// Validate the car ID by checking if the length is 9 characters, the first 2 characters are letters, the next 4 characters are alphanumeric, and the remaining 3 are numbers
bool Inventory::ValidateCarID(string carID, string& errorMessage) {
	bool isValidID = false;
	bool isValidLength = true;
	bool validReq1 = true, validReq2 = true, validReq3 = true;
	string errorMessageTemp{ "" };

	if (carID.length() != REQ_ID_LEN) {
		errorMessageTemp += "Car ID must be " + to_string(REQ_ID_LEN) + " characters long ";
		isValidLength = false;
	}
	else {
		for (int i{ 0 }; i < REQ_ID_LEN; i++) {
			char c = carID[i];
			// Check that the first requirement is satisfied
			if (i < ID_PART1_END) {
				if (!(c >= 'A' && c <= 'Z' && c != 'O') && validReq1 == true) {
					errorMessageTemp += "First 2 characters of Car ID must be letters alpha only (A-Z, letter O is not allowed) ";
					validReq1 = false;
				}
			} // Check that the second requirement is satisfied
			else if (i <= ID_PART2_END && validReq2 == true) {
				if (!((c >= 'A' && c <= 'Z' && c != 'O') || (c >= '0' && c <= '9')) && validReq2 == true) {
					errorMessageTemp += "Characters 3-6 of Car ID must be alphanumeric (A-Z, 0-9, letter 0 is not allowed) ";
					validReq2 = false;
				}
			} // Check that the final requirement is satisfied
			else {
				if (!(c >= '0' && c <= '9') && validReq3 == true) {
					errorMessage += "Characters 7-9 of Car ID must be numeric ";
					validReq3 = false;
				}
			}
		}
	}

	if (isValidLength && validReq1 && validReq2 && validReq3) {
		isValidID = true;
	}
	else {
		errorMessage = "Invalid ID:    [ " + errorMessageTemp + "]";
	}
	return isValidID;
}

// Check if the model starts with a capital letter, is at least 3 characters long, and is alphanumeric
bool Inventory::ValidateModel(string model, string& errorMessage) {
	bool validModel = false;
	bool validFirst = true, validLength = true, validAlphanumeric = true;
	int length = model.length();
	string errorMessageTemp{ "" };

	// Check that the model meets the minimum length requirement
	if (length < MIN_MODEL_LEN) {
		validLength = false;
		errorMessageTemp += "Model must be at least " + to_string(MIN_MODEL_LEN) + " characters long";
	}
	else {
		// Check that the model starts with a capital letter that isn't O
		if (model[0] < 'A' || model[0] > 'Z' || model[0] == 'O') {
			validFirst = false;
			errorMessageTemp += "Model must start with a capital letter (A-Z, letter O is not allowed) ";
		}
		else {
			for (int i{ 1 }; i < length; i++) {
				char c = model[i];

				if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) && validAlphanumeric) {
					errorMessageTemp += "Model must must be alphanumeric (A-Z, 0-9, letter 0 is not allowed) ";
					validAlphanumeric = false;
				}
			}
		}
	}

	if (validFirst && validLength && validAlphanumeric) {
		validModel = true;
	}
	else {
		errorMessage = "Invalid Model: [ " + errorMessageTemp + "] ";
	}
	return validModel;
}

// Check if the quantity is greater than or equal to 0
bool Inventory::ValidateQuantity(int quantity, string& errorMessage) {
	bool validQuantity = false;

	if (quantity >= 0) {
		validQuantity = true;
	}
	else {
		errorMessage += "Invalid Quant: [ Quantity must be greater than or equal to 0 ] ";
	}
	return validQuantity;
}

// Check if the price is greater than MIN_PRICE
bool Inventory::ValidatePrice(double price, string& errorMessage) {
	bool validPrice = false;

	if (price > MIN_PRICE) {
		validPrice = true;
	}
	else {
		errorMessage += "Invalid Price: [ Price must be greater than $5000 ]";
	}
	return validPrice;
}

// Sorts the inventory by user-specified field in descending order
void Inventory::SortBy(int field) {
	Car* temp;

	for (int i{ 0 }; i < numCars; i++) {
		int minIndex = i;

		for (int j = i + 1; j < numCars; j++) {
			bool condition = false;

			switch (field) {
			case ID: // Sort by ID
				condition = carPtrs[j]->getCarID() > carPtrs[minIndex]->getCarID();
				break;
			case MODEL: // Sort by Model
				condition = carPtrs[j]->getModel() > carPtrs[minIndex]->getModel();
				break;
			case QUANTITY: // Sort by Quantity
				condition = carPtrs[j]->getQuantity() > carPtrs[minIndex]->getQuantity();
				break;
			case PRICE: // Sort by Price
				condition = carPtrs[j]->getPrice() > carPtrs[minIndex]->getPrice();
				break;
			default:
				cout << "ERROR: Invalid field. Terminating Program\n";
				exit(EXIT_FAILURE);
			}

			if (condition) {
				minIndex = j;
			}
		}
		temp = carPtrs[minIndex];
		carPtrs[minIndex] = carPtrs[i];
		carPtrs[i] = temp;
	}
}

// Takes in a string and converts all alphabetical characters to uppercase
void Inventory::MakeStringUppercase(string& str) {
	for (int i{ 0 }; i < str.length(); i++) {
		str[i] = toupper(str[i]);
	}
}

void Inventory::PrintInventory() {
	stringstream ssHeader;

	ssHeader << fixed << showpoint << setprecision(2) << left
		<< setw(TEXT_WIDTH) << "Vehicle ID"
		<< setw(TEXT_WIDTH) << "Model"
		<< setw(NUM_WIDTH) << right << "Quantity"
		<< setw(NUM_WIDTH) << "Price" << left << "\n\n";

	cout << "VALID ITEMS IN THE INVENTORY\n"
		<< setfill('-') << setw(HEADER_WIDTH) << "-" << "\n" //use setfill print -
		<< setfill(' ') // Reset setfill to default space character
		<< ssHeader.str();

	string recordString{ "" };

	if (carPtrs == nullptr) {
		cout << "Error: Inventory not initialized properly. Terminating Program\n\n";
		exit(EXIT_FAILURE);
	}

	for (int i{ 0 }; i < numCars; i++) {
		recordString = carPtrs[i]->toString();
		cout << recordString;
	}
	cout << "\nTotal Records: " << numCars << "\n"
		<< setfill('-') << setw(HEADER_WIDTH) << "-" << "\n"
		<< setfill(' ') << "\n\n";
}

int GetMenuSelection() {
	int selection;

	cout << "Main Menu:\n"
		"Please select one of the following options:\n"
		"1. Valid Records\n"
		"2. Invalid Records\n"
		"3. Sort Inventory\n"
		"4. Quit program\n"
		"Selection: ";
	cin >> selection;
	cout << endl;

	return selection;
}

void SortMenu(Inventory& inventory) {
	int field = GetSortKey();

	if (field != RETURN_TO_MAIN) {
		switch (field) {
		case ID:
			cout << "Sorting by ID\n\n";
			break;
		case MODEL:
			cout << "Sorting by Model\n\n";
			break;
		case QUANTITY:
			cout << "Sorting by Quantity\n\n";
			break;
		case PRICE:
			cout << "Sorting by Price\n\n";
			break;
		default:
			PurgeInputErrors("ERROR: Invalid case in SortMenu. Terminating Program.\n");
			exit(EXIT_FAILURE);
		}
		inventory.SortBy(field);
		inventory.PrintInventory();
	}
	else {
		cout << "Returning to Main Menu\n\n";
	}
}

int GetSortKey() {
	int key;

	do {
		cout << "Sort Menu:\n"
			"Please select one of the following sorting options:\n"
			"1. ID\n"
			"2. Model\n"
			"3. Quantity\n"
			"4. Price\n"
			"5. Return to Main Menu\n"
			"Selection: ";

		cin >> key;
		cout << "\n";

		if (key < ID || key > RETURN_TO_MAIN) {
			PurgeInputErrors("Error: Invalid menu selection\n\n");
		}
	} while (key < ID || key > RETURN_TO_MAIN);

	return key;
}

void PrintInvalidRecords() {
	string errorFileName{ "ErrorFile.txt" };
	ifstream Errfile(errorFileName);
	stringstream ssHeader;
	int invalidRecordCount{ 0 };

	cout << fixed << showpoint << setprecision(2);
	ssHeader << fixed << showpoint << setprecision(2) << left
		<< setw(TEXT_WIDTH) << "Vehicle ID"
		<< setw(TEXT_WIDTH) << "Model"
		<< setw(NUM_WIDTH) << right << "Quantity"
		<< setw(NUM_WIDTH) << "Price" << left << "\n\n";

	if (!Errfile.is_open()) {
		cout << "ERROR: Unable to open file.\n\n";
		exit(EXIT_FAILURE);
	}
	else if (Errfile.peek() == EOF) {
		cout << "No Invalid Records Found.\n\n";
	}
	else {
		cout << "INVALID RECORDS\n"
			<< setfill('-') << setw(HEADER_WIDTH) << "-" << "\n"
			<< setfill(' ') << ssHeader.str();

		string line;
		while (getline(Errfile, line)) {
			cout << line << endl;
		}
		cout << "\nTotal Records: " << invalidRecordCount << "\n"
			<< setfill('-') << setw(HEADER_WIDTH) << "-" << "\n"
			<< setfill(' ') << "\n\n";

		Errfile.close();
	}
}

// Clears the error state of cin and ignores any remaining invalid input in the buffer
void PurgeInputErrors(string errMess) {
	cout << errMess;
	if (cin.fail()) {
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
}

/*
TEST DATA

AB12MP349 Fusion5 20 17000.00 (Valid)
35KMOP324 Civic 7 11999.00 (Invalid Car ID: Contains 'O' in Car ID)
AB12MP34 RX5 1 17000.00 (Invalid Car ID: Incorrect length)
AB12MP349 gwaggon 17000.00 (Invalid Model Name: Contains non-alphanumeric characters)
XY34LK678 Mustang7 15 21500.00 (Valid)
KL67NM123 accord3 8 9800.00 (Invalid Model Name: Starting with lowercase)
PQ78RS901 Camry9 0 7500.00 (Valid)
JK90AB234 RX510 -3 7500.00 (Invalid Quantity: Negative quantity)
DE56FG789 Corolla4 12 6900.00 (Valid)
MN12CD567 Sonata6 5 6700.00 (Valid)
UV45EF890 Optima4 0 5200.00 (Valid)
UV45EF891 Optima5 0 5300.00 (Valid)
UV45EF892 Optima6 0 5400.00 (Valid)
GH23IJ457 Civic6 10 14500.00 (Valid)
GH23IJ458 Civic7 8 15000.00 (Valid)
ZA89BC124 Prius8 7 12000.00 (Valid)
ZA89BC125 Prius9 5 12500.00 (Valid)
AB12MP350 Fusion6 9 17500.00 (Valid)
AB12MP351 Fusion7 7 18000.00 (Valid)
XY34LK679 Mustang8 12 22500.00 (Valid)
XY34LK680 Mustang9 10 23000.00 (Valid)
KL67NM126 Accord6 6 11000.00 (Valid)
KL67NM127 Accord7 4 11500.00 (Valid)
PQ78RS904 Camry12 3 8100.00 (Valid)
PQ78RS905 Camry13 2 8200.00 (Valid)
JK90AB237 RX513 4 7900.00 (Valid)
JK90AB238 RX514 3 8000.00 (Valid)
DE56FG792 Corolla7 5 7300.00 (Valid)
DE56FG793 Corolla8 3 7400.00 (Valid)
MN12CD570 Sonata9 6 7100.00 (Valid)
MN12CD571 Sonata10 4 7200.00 (Valid)
ZA89BC123 Prius7 10 11000.00 (Valid)
35KMOP324 Fusion5 10 6700.00 (Invalid Car ID: Contains 'O' in Car ID)
AB12MP349 R 15 8800.00 (Invalid Model Name: Less than 3 characters)
XY34LK678 Mustang7 0 21500.00 (Invalid Quantity: Zero quantity)
KL67NM123 Accord3 6 3500.00 (Invalid Price: Below $5,000.00)
9PQRST123 MercC 7 9800.00 (Invalid Model Name: Contains non-alphanumeric characters)
MN12CD567 Sonata6 2 12000 (Invalid Price: Missing decimal)
UV45EF890 P 0 5200.00 (Invalid Model Name: Less than 3 characters)
AB12MP349 Fusion5 20 17000.00 (Valid)
XY34LK678 Mustang7 15 21500.00 (Valid)
KL67NM123 Accord3 8 9800.00 (Valid)
PQ78RS901 Camry9 0 7500.00 (Valid)
JK90AB234 RX510 10 7500.00 (Valid)
DE56FG789 Corolla4 12 6900.00 (Valid)
MN12CD567 Sonata6 5 6700.00 (Valid)
UV45EF890 Optima4 0 5200.00 (Valid)
GH23IJ456 Civic5 18 13500.00 (Valid)
ZA89BC123 Prius7 10 11000.00 (Valid)
KL67NM124 Accord4 5 10500.00 (Valid)
KL67NM125 Accord5 3 11500.00 (Valid)
PQ78RS902 Camry10 2 7600.00 (Valid)
PQ78RS903 Camry11 0 8000.00 (Valid)
JK90AB235 RX511 7 7700.00 (Valid)
JK90AB236 RX512 6 7800.00 (Valid)
DE56FG790 Corolla5 4 7100.00 (Valid)
DE56FG791 Corolla6 2 7200.00 (Valid)
MN12CD568 Sonata7 5 6900.00 (Valid)
MN12CD569 Sonata8 3 7000.00 (Valid)
GH23IJ456 Pass@t 1 1200.00 (Invalid Model Name: Contains non-alphanumeric characters)
ZA89BC123 Prius4 10 4200.00 (Valid)
JK78EF901 MustangGT 3 22000.00 (Valid)
LM12CD345 CorollaLE 12 15000.00 (Valid)
HJ23IJ486 Civic8 4 17700.00 (Valid)
XY56GH789 Wrangler4X4 8 28750.00 (Valid)
AB34JK567 CamryXSE 5 32000.00 (Valid)
OP67UV890 CivicSi 20 21000.00 (Valid)
PQ90RS123 SonataSEL 0 -17800.00 (Invalid Price: Negative price)
KL23MN456 ExplorerXLT 15 27000.00 (Valid)
EF45AB678 OptimaEX 10 23000.00 (Valid)
AZ12BC345 MustangGT500 1 75000.00 (Valid)
ZZ99YY123 CorvetteZ06 25 102000.00 (Valid)
AB12CD345 Corolla-SE 5 16500.00 (Invalid Model Name: Contains hyphen)
EF56GH789 ElectricCar 0 28000.00 (Valid)
OP67UV890 Sedan123 20 5000.00 (Valid)
KL23MN456 4RunnerLimited 8 50000.00 (Valid)
EF45AB678 LongModelName 10 5000.01 (Valid)
AB55AB678 OptimaEX 10 2300.00 (Invalid Price: Below $5,000.00)


----------- RUN PROGRAM THEN COPY PASTE OUTPUT HERE ---------------
*****Test Case #1: User inputs an invalid option in Main Menu*****

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: asdf

ERROR: Invalid input. Please enter a valid option.

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: -15

ERROR: Invalid input. Please enter a valid option.

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection:


*****Test Case #2: User inputs an invalid option in Sort Menu*****

Main Menu:
Please select one of the following options:
1. Valid Records
2. Invalid Records
3. Sort Inventory
4. Quit program
Selection: 3

Sort Menu:
Please select one of the following sorting options:
1. ID
2. Model
3. Quantity
4. Price
5. Return to Main Menu
Selection: asdfa

ERROR: Invalid menu selection. Please enter a valid option.

Sort Menu:
Please select one of the following sorting options:
1. ID
2. Model
3. Quantity
4. Price
5. Return to Main Menu
Selection: asdfa


*****Test Case #3: The data file is empty*****

ERROR: 'Data.txt' is empty. No data to process. Terminating Program


*****Test Case #4: The data file contains only invalid records*****

Main Menu:
Please select one of the following options:
1. Valid Records
2. Invalid Records
3. Sort Inventory
4. Quit program
Selection: 1

VALID ITEMS IN THE INVENTORY
------------------------------------------------------
Vehicle ID     Model              Quantity       Price


Total Records: 0
------------------------------------------------------

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: 2

INVALID RECORDS
------------------------------------------------------
Vehicle ID     Model              Quantity       Price

X234LK678      Mustang7                  0       21500 Invalid ID:    [ First 2 characters of Car ID must be letters alpha only (A-Z, letter O is not allowed) ]
KL670M123      Accord3                   6        3500 Invalid Price: [ Price must be greater than $5000 ]
9PQRST123      MercC                     7        9800 Invalid ID:    [ First 2 characters of Car ID must be letters alpha only (A-Z, letter O is not allowed) ]

Total Records: 3
------------------------------------------------------

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: 3

Terminating Program


*****Test Case #4: The data file contains only valid records*****
Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: 1

VALID ITEMS IN THE INVENTORY (UNSORTED)
------------------------------------------------------
Vehicle ID     Model              Quantity       Price

AB12MP349      Fusion5                  20    17000.00
KL23MN456      ExplorerXLT              15    27000.00
AZ12BC345      MustangGT500              1    75000.00
ZZ99YY123      CorvetteZ06              25   102000.00
EF56GH789      ElectricCar               0    28000.00
EF45AB678      LongModelName            10     5000.01

Total Records: 6
------------------------------------------------------

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection: 2

No Invalid Records Found.

Menu:
Please select one of the following options:
1. Print all valid items in the inventory (unsorted)
2. Print invalid records
3. Quit program
Selection:


*****Test Case #5: No matching vehicle records - Search by ID/Model*****

Enter the Vehicle ID or Model Name: ab

No records found matching the search criteria.

Search Menu:
Please select one of the following options:
1. Search by ID/Model
2. Search by Price (+-15%)
3. Return to Main Menu
4. Exit Program
Selection:


*****Test Case #6: No matching vehicle records - Search by Price*****

Search Menu:
Please select one of the following options:
1. Search by ID/Model
2. Search by Price (+-15%)
3. Return to Main Menu
4. Exit Program
Selection: 2

Enter the price to search for: $9999999999

No records found matching the search criteria.

Search Menu:
Please select one of the following options:
1. Search by ID/Model
2. Search by Price (+-15%)
3. Return to Main Menu
4. Exit Program
Selection:


*****Test Case #7: Case sensitivity in Search by Model/ID*****
*
Search Menu:
Please select one of the following options:
1. Search by ID/Model
2. Search by Price (+-15%)
3. Return to Main Menu
4. Exit Program
Selection: 1

Enter the Vehicle ID or Model Name: civic

RECORDS CONTAINING 'CIVIC'
------------------------------------------------------
Vehicle ID     Model              Quantity       Price

GH23IJ457      CIVIC6                   10    14500.00
GH23IJ458      CIVIC7                    8    15000.00
GH23IJ456      CIVIC5                   18    13500.00
HJ23IJ486      CIVIC8                    4    17700.00

Total Records: 4
------------------------------------------------------


*****Test Case #8: Invalid input in search by price*****

Search Menu:
Please select one of the following options:
1. Search by ID/Model
2. Search by Price (+-15%)
3. Return to Main Menu
4. Exit Program
Selection: 2

Enter the price to search for: $asdf

ERROR: 'asdf' is not a valid Price

*/