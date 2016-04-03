#include "structure.h"

#define SUCCESS_MESSAGE         "OK\n"
#define IGNORE_MESSAGE          "IGNORED\n"
#define DESCRIPTION_MESSAGE     "DESCRIPTIONS: %d\n"

typedef struct hospitalData hospitalData;
typedef struct patient patient;
typedef struct diseaseStructure diseaseStructure;
typedef struct diseaseListNode diseaseListNode;

struct hospitalData {
    patient *firstPatient;
    int storedDiseasesCount;
};

// Global structure keeps number of descriptions and list of patients.
hospitalData hospitalGlobalData;

void initializeHospitalGlobalData(void) {
    // Initialize global structure with dummy ahead list.
    hospitalGlobalData.storedDiseasesCount = 0;
    hospitalGlobalData.firstPatient = malloc(sizeof(patient));

    // Initialize dummy.
    hospitalGlobalData.firstPatient->diseaseListLast= NULL;
    hospitalGlobalData.firstPatient->diseaseListHead = NULL;
    hospitalGlobalData.firstPatient->nextPatient = NULL;
    hospitalGlobalData.firstPatient->patientName = NULL;
}

void printIgnoredUponFailure(void) {
    printf(IGNORE_MESSAGE);
}

void printOKUponSuccess(void) {
    printf(SUCCESS_MESSAGE);
}

void debugModePrintDescriptions(void) {
    fprintf(stderr,
            DESCRIPTION_MESSAGE,
            hospitalGlobalData.storedDiseasesCount);
}

void decreaseCountAndDeleteIfNotReferred(diseaseStructure *diseaseReference) {
    diseaseReference->referenceCount--;

    // If disease is not referred by anyone then it is freed.
    if (diseaseReference->referenceCount == 0) {
        // diseaseReference and diseaseReference->diseaseDescription
        // is never null.
        free(diseaseReference->diseaseDescription);
        diseaseReference->diseaseDescription = NULL;
        free(diseaseReference);
        diseaseReference = NULL;
        // Decrease global descriptions count.
        hospitalGlobalData.storedDiseasesCount--;
    }
}

patient* findPatientPrecedingGivenName(char *patientName) {
    patient *currentPatient = hospitalGlobalData.firstPatient;

    // Finds patient that nextPatient->patientName is either our patient or
    // null pointer which means patient with patientName was not found in
    // database.
    while (currentPatient->nextPatient != NULL && // Lazy evaluation.
           strcmp(currentPatient->nextPatient->patientName, patientName) != 0) {
        currentPatient = currentPatient->nextPatient;
    }

    // Returns pointer to patient before our patient (or null).
    return currentPatient;
}

patient* getPatientPointerAllocateIfNull(char *patientName,
                                         patient *currentPatient) {
    // Checks if we are at the end of the list or next patient on the list is
    // not our patient. If so allocates and initializes newPatient.
    if (currentPatient->nextPatient == NULL || // Lazy evaluation.
            strcmp(currentPatient->nextPatient->patientName, patientName) != 0) {
        patient *newPatient = malloc(sizeof(patient));

        newPatient->patientName = patientName;
        // Initializes dummy and sets diseaseListHead to point that dummy.
        newPatient->diseaseListHead = malloc(sizeof(diseaseListNode));
        newPatient->diseaseListLast = newPatient->diseaseListHead;
        newPatient->diseaseListLast->nextDisease = NULL;
        newPatient->diseaseListHead->diseaseReference = NULL;

        newPatient->nextPatient = currentPatient->nextPatient;
        currentPatient->nextPatient = newPatient;
    } else {
        // Patient with patientName exists in out database and string
        // patientName is no longer needed and has to be freed.
        free(patientName);
        patientName = NULL;
    }

    // Returns pointer to patient that patientName is same as our
    // char *patientName.
    return currentPatient->nextPatient;
}

diseaseStructure* createNewDiseaseStructure(char *diseaseDescription) {
    diseaseStructure *newDisease = malloc(sizeof(diseaseStructure));

    newDisease->diseaseDescription = diseaseDescription;
    newDisease->referenceCount = 1;

    hospitalGlobalData.storedDiseasesCount++;

    return newDisease;
}

void createNewDiseaseListNode(patient *currentPatient,
                              char *diseaseDescription) {
    // Allocates memory for diseaseListNode and diseaseStructure.
    diseaseListNode *newDiseaseNode = malloc(sizeof(diseaseListNode));
    diseaseStructure *newDisease = NULL;

    // Initializes diseaseStructure with arguments passed to the function.
    newDisease = createNewDiseaseStructure(diseaseDescription);

    // Initializes diseaseListNode with arguments passed to the function.
    newDiseaseNode->nextDisease = currentPatient->diseaseListLast->nextDisease;
    currentPatient->diseaseListLast->nextDisease = newDiseaseNode;
    currentPatient->diseaseListLast = newDiseaseNode;
    newDiseaseNode->diseaseReference = newDisease;
}

void addNewDisease(char *patientName,
                   char *diseaseDescription) {
    patient* currentPatient = NULL;
    // We are inserting patients into our list without any order because it
    // does not change our time complexity.

    // Finds patient with patientName field preceding patientName given as
    // argument. If patient with patientName does not exist in the database
    // then pointer to the last patient on the list is given.
    patient *precedingPatient = findPatientPrecedingGivenName(patientName);

    // Gets pointer to patient with patientName same as given argument
    // (allocates and initializes new structure if patient doesn't exists).
    currentPatient = getPatientPointerAllocateIfNull(patientName,
                                                     precedingPatient);

    // Allocates and initializes new structure for disease and inserts that
    // disease at the end of patient diseaseList.
    createNewDiseaseListNode(currentPatient,
                             diseaseDescription);
    printOKUponSuccess();
}

// Adds disease that currently exists.
void addDiseaseToListAfterListNode(diseaseListNode *precedingDiseaseListNode,
                                   diseaseStructure *diseaseAdded) {
    diseaseListNode *newDiseaseListNode = malloc(sizeof(diseaseListNode));

    newDiseaseListNode->nextDisease = precedingDiseaseListNode->nextDisease;
    precedingDiseaseListNode->nextDisease = newDiseaseListNode;
    newDiseaseListNode->diseaseReference = diseaseAdded;
    diseaseAdded->referenceCount++;
}

void copyLatestDisease(char *toPatientName,
                       char *fromPatientName) {
    patient *fromPatient = NULL;
    patient *toPatient = NULL;

    patient *patientPrecedingFromPatient = NULL;
    patient *patientPrecedingToPatient = NULL;

    diseaseStructure *diseaseCopied = NULL;

    patientPrecedingFromPatient = findPatientPrecedingGivenName(fromPatientName);
    fromPatient = patientPrecedingFromPatient->nextPatient;

    if (fromPatient == NULL || // Lazy evaluation.
            fromPatient->diseaseListHead == fromPatient->diseaseListLast) {
        // If diseaseListHead equals diseaseListLast then our list of
        // diseases is empty (we keep dummy ahead that list).
        printIgnoredUponFailure();
        free(toPatientName);
        toPatientName = NULL;
    } else {
        patientPrecedingToPatient = findPatientPrecedingGivenName(toPatientName);
        toPatient = getPatientPointerAllocateIfNull(toPatientName,
                                                    patientPrecedingToPatient);
        diseaseCopied = fromPatient->diseaseListLast->diseaseReference;

        addDiseaseToListAfterListNode(toPatient->diseaseListLast,
                                      diseaseCopied);
        toPatient->diseaseListLast = toPatient->diseaseListLast->nextDisease;
        printOKUponSuccess();
    }

    free(fromPatientName);
    fromPatientName = NULL;
}

// Returns NULL when diseaseListNode was not found.
diseaseListNode* findDiseaseListNode (patient *currentPatient,
                                      int diseaseID) {
    diseaseListNode* diseaseListNodeToReturn = NULL;
    diseaseListNode* currentDiseaseListNode = currentPatient->diseaseListHead;

    int diseaseCount;

    for (diseaseCount = 0; diseaseCount < diseaseID &&
            currentDiseaseListNode != NULL; diseaseCount++) {
        currentDiseaseListNode = currentDiseaseListNode->nextDisease;
    }

    if (diseaseCount == diseaseID) {
        diseaseListNodeToReturn = currentDiseaseListNode;
    }

    return diseaseListNodeToReturn;
}

void changePatientDiseaseDescription(char *patientName,
                                     int diseaseID,
                                     char *diseaseDescription) {
    patient *currentPatient = NULL;
    patient *patientPrecedingCurrentPatient = NULL;

    diseaseListNode *diseaseListNodeToModify = NULL;
    diseaseStructure *diseaseToModify = NULL;
    diseaseStructure *newDisease = NULL;

    patientPrecedingCurrentPatient = findPatientPrecedingGivenName(patientName);
    currentPatient = patientPrecedingCurrentPatient->nextPatient;

    if (currentPatient == NULL || diseaseID < 1) {
        // Patient was not found in database or
        // tried referring to disease with wrong ID.
        printIgnoredUponFailure();
        free(diseaseDescription);
        diseaseDescription = NULL;
    } else {
        diseaseListNodeToModify = findDiseaseListNode(currentPatient,
                                                      diseaseID);
        if (diseaseListNodeToModify == NULL) {
            printIgnoredUponFailure();
            free(diseaseDescription);
            diseaseDescription = NULL;
        } else {
            diseaseToModify = diseaseListNodeToModify->diseaseReference;
            decreaseCountAndDeleteIfNotReferred(diseaseToModify);
            newDisease = createNewDiseaseStructure(diseaseDescription);
            diseaseListNodeToModify->diseaseReference = newDisease;
            printOKUponSuccess();
        }
    }

    free(patientName);
    patientName = NULL;
}

void printDiseaseDescription(char *patientName,
                              int diseaseID) {
    patient *currentPatient = NULL;
    patient *patientPrecedingCurrentPatient = NULL;

    diseaseListNode *diseaseNodeToPrint = NULL;

    patientPrecedingCurrentPatient = findPatientPrecedingGivenName(patientName);
    currentPatient = patientPrecedingCurrentPatient->nextPatient;

    if (currentPatient == NULL || diseaseID < 1) {
        // Patient was not found in database or
        // tried referring to disease with wrong ID.
        printIgnoredUponFailure();
    } else {
        diseaseNodeToPrint = findDiseaseListNode(currentPatient,
                                                 diseaseID);
        if (diseaseNodeToPrint == NULL) {
            printIgnoredUponFailure();
        } else {
            printf("%s\n",
                   diseaseNodeToPrint->diseaseReference->diseaseDescription);
        }
    }

    free(patientName);
    patientName = NULL;
}

void deletePatientDiseaseList(patient *currentPatient) {
    if (currentPatient->diseaseListHead != NULL) {
        diseaseListNode *nextDiseaseListNodeToRemove =
                currentPatient->diseaseListHead->nextDisease;
        diseaseListNode *temporaryDiseaseListNode = NULL;

        // Frees diseaseStructure from patient's diseaseListHead one by one.
        while (nextDiseaseListNodeToRemove != NULL) {
            temporaryDiseaseListNode = nextDiseaseListNodeToRemove;
            decreaseCountAndDeleteIfNotReferred(
                    temporaryDiseaseListNode->diseaseReference);
            temporaryDiseaseListNode->diseaseReference = NULL;
            nextDiseaseListNodeToRemove = nextDiseaseListNodeToRemove->nextDisease;
            temporaryDiseaseListNode->nextDisease = NULL;
            free(temporaryDiseaseListNode);
            temporaryDiseaseListNode = NULL;
        }

        currentPatient->diseaseListHead->nextDisease = NULL;
        currentPatient->diseaseListLast = currentPatient->diseaseListHead;
    }
}

void deletePatientData(patient *patientBeingRemoved) {
    deletePatientDiseaseList(patientBeingRemoved);
    // Frees dummy from head of the list.
    if (patientBeingRemoved != NULL) {
        if (patientBeingRemoved->diseaseListHead != NULL) {
            free(patientBeingRemoved->diseaseListHead);
            patientBeingRemoved->diseaseListHead = NULL;
            patientBeingRemoved->diseaseListLast = NULL;
        }
        if (patientBeingRemoved->patientName != NULL) {
            free(patientBeingRemoved->patientName);
            patientBeingRemoved->patientName = NULL;
        }
        free(patientBeingRemoved);
        patientBeingRemoved = NULL;
    }
}

void deletePatientDiseaseData(char *patientName) {
    patient *currentPatient = NULL;
    patient *patientPrecedingCurrentPatient = NULL;

    patientPrecedingCurrentPatient = findPatientPrecedingGivenName(patientName);
    currentPatient = patientPrecedingCurrentPatient->nextPatient;

    if (currentPatient == NULL) {
        // Patient was not found in database.
        printIgnoredUponFailure();
    } else {
        deletePatientDiseaseList(currentPatient);
        printOKUponSuccess();
    }

    free(patientName);
    patientName = NULL;
}

void freeAllocatedMemory(void) {
    // Frees list of patients from global structure.
    patient *nextPatientToRemove = hospitalGlobalData.firstPatient;
    patient *temporaryPatient = NULL;
    while (nextPatientToRemove != NULL) {
        temporaryPatient = nextPatientToRemove;
        nextPatientToRemove = nextPatientToRemove->nextPatient;
        deletePatientData(temporaryPatient);
    }
}

void performOperation(int operationCode,
                      int *integerArgument,
                      char *stringArgument1,
                      char *stringArgument2) {
    switch (operationCode) {
        case -1:
            break;
        case 1:
            addNewDisease(stringArgument1,
                          stringArgument2);
            break;
        case 2:
            copyLatestDisease(stringArgument1,
                              stringArgument2);
            break;
        case 3:
            changePatientDiseaseDescription(stringArgument1,
                                            *integerArgument,
                                            stringArgument2);
            break;
        case 4:
            printDiseaseDescription(stringArgument1,
                                    *integerArgument);
            break;
        case 5:
            deletePatientDiseaseData(stringArgument1);
            break;
        case 0:
        default:
            printIgnoredUponFailure();
    }
}