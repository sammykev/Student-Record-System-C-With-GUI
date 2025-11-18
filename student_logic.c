#include "student_logic.h"
#include <stdlib.h>
#include <string.h>

// --- List Management ---

void initList(StudentList *list) {
    list->students = NULL;
    list->count = 0;
    list->capacity = 0;
}

void freeList(StudentList *list) {
    free(list->students);
    list->students = NULL;
    list->count = 0;
    list->capacity = 0;
}

void ensureCapacity(StudentList *list) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->students = realloc(list->students, list->capacity * sizeof(Student));
        if (list->students == NULL) {
            fprintf(stderr, "Memory allocation failed!\n"); // Error to stderr
            exit(EXIT_FAILURE);
        }
    }
}

// --- Core Data Operations ---

int addStudent(StudentList *list, const char* name, int roll, float marks) {
    // Check if roll number already exists
    if (searchStudent(list, roll) != -1) {
        return 0; // Failure: Roll number already exists
    }
    
    ensureCapacity(list);
    Student s;
    strncpy(s.name, name, NAME_LEN - 1);
    s.name[NAME_LEN - 1] = '\0'; // Ensure null termination
    s.roll = roll;
    s.marks = marks;

    list->students[list->count++] = s;
    return 1; // Success
}

int modifyStudent(StudentList *list, int roll, const char* newName, float newMarks) {
    int idx = searchStudent(list, roll);
    if (idx == -1) {
        return 0; // Failure: Student not found
    }

    // Only update if newName is not empty
    if (newName != NULL && strlen(newName) > 0) {
        strncpy(list->students[idx].name, newName, NAME_LEN - 1);
        list->students[idx].name[NAME_LEN - 1] = '\0';
    }
    
    // Only update if newMarks is not the signal value (-1)
    if (newMarks >= 0) {
        list->students[idx].marks = newMarks;
    }

    return 1; // Success
}

int removeStudent(StudentList *list, int roll) {
    int idx = searchStudent(list, roll);
    if (idx == -1) {
        return 0; // Failure: Student not found
    }
    for (int i = idx; i < list->count - 1; i++) {
        list->students[i] = list->students[i + 1];
    }
    list->count--;
    return 1; // Success
}

int searchStudent(const StudentList *list, int roll) {
    for (int i = 0; i < list->count; i++) {
        if (list->students[i].roll == roll)
            return i;
    }
    return -1; // Not found
}

// --- Data Processing ---

void sortStudents(StudentList *list, int ascending) {
    for (int i = 0; i < list->count - 1; i++) {
        for (int j = i + 1; j < list->count; j++) {
            int condition = ascending
                ? (list->students[i].marks > list->students[j].marks)
                : (list->students[i].marks < list->students[j].marks);
            
            if (condition) {
                Student temp = list->students[i];
                list->students[i] = list->students[j];
                list->students[j] = temp;
            }
        }
    }
    // No printf message! The GUI/console will handle that.
}

float getAverageMarks(const StudentList *list) {
    if (list->count == 0) {
        return 0.0f; // Return 0 if no students
    }
    float sum = 0;
    for (int i = 0; i < list->count; i++) {
        sum += list->students[i].marks;
    }
    return sum / list->count;
}


// --- File I/O ---

int saveToFile(const StudentList *list) {
    // Note: Your original used "./records.txt", but the #define used "students.txt"
    // I'll use the FILENAME define from the header.
    FILE *fp = fopen(FILENAME, "w");  
    if (!fp) {
        return 0; // Failure
    }
    for (int i = 0; i < list->count; i++) {
        fprintf(fp, "%s,%d,%.2f\n", list->students[i].name, list->students[i].roll, list->students[i].marks);
    }
    fclose(fp);
    return 1; // Success
}

int loadFromFile(StudentList *list) {
    FILE *fp = fopen(FILENAME, "r");
    if (!fp) {
        return 0; // Failure
    }
    
    freeList(list); // Clear the current list before loading
    initList(list);
    
    char line[NAME_LEN + 30];
    while (fgets(line, sizeof(line), fp)) {
        char name[NAME_LEN];
        int roll;
        float marks;

        char *token = strtok(line, ",");
        if (token) strncpy(name, token, NAME_LEN);
        token = strtok(NULL, ",");
        if (token) roll = atoi(token);
        token = strtok(NULL, ",");
        if (token) marks = atof(token);
        
        // Use our safe addStudent function
        addStudent(list, name, roll, marks);
    }
    fclose(fp);
    return 1; // Success
}