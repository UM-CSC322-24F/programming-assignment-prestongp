#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define NAME_LENGTH 127

typedef enum {
    slip, land, trailor, storage
	} PlaceType;

typedef union {
    int slipNum;
    char bayLetter;
    char licenseTag[10];
    int storageSpace;
	} ExtraInfo;

typedef struct {
    char name[NAME_LENGTH + 1];
    float length;
    PlaceType place;
    ExtraInfo extra;
    float owed;
	} Boat;

Boat *boats[MAX_BOATS] = {NULL};
int numBoats = 0;

int compare(const void *a, const void *b) {
    const Boat *boatA = *(const Boat **)a;
    const Boat *boatB = *(const Boat **)b;
    return strcmp(boatA -> name, boatB -> name);
}

void loadData(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        Boat *boat = malloc(sizeof(Boat));
        if (!boat) {
            perror("Memory allocation failed");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        char type[10];
        sscanf(line, "%[^,],%f,%[^,],%[^,],%f", 
               boat->name, &boat->length, type, 
               boat->extra.licenseTag, &boat->owed);

        // Set the `place` and `extra` data based on `type`
        if (strcmp(type, "slip") == 0) {
            boat->place = slip;
            boat->extra.slipNum = atoi(boat->extra.licenseTag);
        } else if (strcmp(type, "land") == 0) {
            boat->place = land;
            boat->extra.bayLetter = boat->extra.licenseTag[0];
        } else if (strcmp(type, "trailor") == 0) {
            boat->place = trailor;
        } else if (strcmp(type, "storage") == 0) {
            boat->place = storage;
            boat->extra.storageSpace = atoi(boat->extra.licenseTag);
        }

        boats[numBoats++] = boat;
    }

    fclose(file);
}

void save(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < numBoats; i++) {
        Boat *boat = boats[i];

        switch (boat->place) {
            case slip:
                fprintf(file, "%s,%.0f,slip,%d,%.2f\n",boat->name, boat->length, boat->extra.slipNum, boat->owed);
                break;
            case land:
                fprintf(file, "%s,%.0f,land,%c,%.2f\n", boat->name, boat->length, boat->extra.bayLetter, boat->owed);
                break;

            case trailor:
                fprintf(file, "%s,%.0f,trailor,%s,%.2f\n", boat->name, boat->length, boat->extra.licenseTag, boat->owed);
                break;
            case storage:
                fprintf(file, "%s,%.0f,storage,%d,%.2f\n", boat->name, boat->length, boat->extra.storageSpace, boat->owed);
                break;
            default:
                printf("Unknown place type for boat: %s\n", boat->name);
                break;
        }
    }

    fclose(file);
}

void add(const char *csv_line) {
	if (numBoats == MAX_BOATS) {
		printf("Error: maximum number of boats reached\n");
		return;
	}
	Boat *boat = (Boat *)malloc(sizeof(Boat));
	char type[10];

	sscanf(csv_line, "%[^,],%f,%[^,],%[^,],%f", boat->name, &boat->length, type, boat->extra.licenseTag, &boat->owed);

	if (strcmp(type, "slip") == 0) {
    		boat->place = slip;
    		boat->extra.slipNum = atoi(boat->extra.licenseTag);
	} else if (strcmp(type, "land") == 0) {
    		boat->place = land;
    		boat->extra.bayLetter = boat->extra.licenseTag[0];
	} else if (strcmp(type, "trailor") == 0) {
    		boat->place = trailor;
	} else if (strcmp(type, "storage") == 0) {
	    	boat->place = storage;
	    	boat->extra.storageSpace = atoi(boat->extra.licenseTag);
	}

	boats[numBoats++] = boat;
}

void boatPrint() {
	printf("%-20s %-20s %-20s %-20s %-20s\n", "Name", "Length", "Place", "Extra Info", "Amount Owed");
	for (int i = 0; i < numBoats; i++) {
		Boat *boat = boats[i];
		printf("%-20s %-20.0f ", boat->name, boat->length);
	
		switch (boat->place) {
			case slip:
			printf("%-20s %-20d ", "Slip", boat->extra.slipNum);
			break;
		case land:
			printf("%-20s %-20c ", "Land", boat->extra.bayLetter);
			break;
		case trailor:
			printf("%-20s %-20s ", "Trailor", boat->extra.licenseTag);
			break;
		case storage:
			printf("%-20s %-20d ", "Storage", boat->extra.storageSpace);
			break;
			}

		printf("%-20.2f\n", boat->owed);

		}
	}

void freeMem() {
for (int i = 0; i < numBoats; i++) {
free(boats[i]);
boats[i] = NULL;
}
numBoats = 0;
}

void remove_boat(const char *name) {
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            free(boats[i]);
            boats[i] = boats[--numBoats];
            qsort(boats, numBoats, sizeof(Boat *), compare);
            printf("Boat '%s' removed.\n", name);
            return;
        }
    }
    printf("No boat with that name.\n");
}

void payment(const char *name, float amount) {
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            if (amount > boats[i]->owed) {
                printf("That is more than the amount owed, $%.2f\n", boats[i]->owed);
            } else {
                boats[i]->owed -= amount;
                printf("Payment processed.\n");
            }
            return;
        }
    }
    printf("No boat with that name.\n");
}

void updateAmount() {
    for (int i = 0; i < numBoats; i++) {
        boats[i]->owed *= 1.05;
    }
    printf("Monthly fees applied.\n");
}

int main() {
    char input[128], name[NAME_LENGTH + 1];
    float amount;

    loadData("BoatData.csv");

    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it   : ");
        scanf(" %s", input);

        switch (tolower(input[0])) {
            case 'i':
		qsort(boats, numBoats, sizeof(Boat *), compare); 
                boatPrint();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format                   : ");
                scanf(" %[^\n]", input);
                add(input);
                break;
            case 'r':
                printf("Please enter the boat name                                 : ");
                scanf(" %[^\n]", name);
                remove_boat(name);
                break;
            case 'p':
                printf("Please enter the boat name                                 : ");
                scanf(" %[^\n]", name);
                printf("Please enter the amount to be paid                         : ");
                scanf("%f", &amount);
                payment(name, amount);
                break;
            case 'm':
                updateAmount();
                break;
            case 'x':
		printf("Exiting the Boat Management System\n");
		save("newboats.csv");
                freeMem();
                return 0;
            default:
                printf("Invalid option %s\n", input);
                break;
        }
    }
}
