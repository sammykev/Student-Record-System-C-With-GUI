#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student_logic.h" // <-- Include our new header!

// --- Console-Specific Helper Functions ---

void greetUser() {
    char userName[NAME_LEN];
    printf("Welcome to the Student Record System!\n");
    printf("Please enter your name: ");
    fgets(userName, NAME_LEN, stdin);
    userName[strcspn(userName, "\n")] = 0; // Remove trailing newline
    printf("Hello, %s! Let's manage some student records.\n", userName);
}

void displayStudentsConsole(const StudentList *list) {
    if (list->count == 0) {
        printf("No student records to display.\n");
        return;
    }

    printf("\n%-20s %-10s %-10s %-10s\n", "Name", "Roll No", "Marks", "Status");
    for (int i = 0; i < list->count; i++) {
        printf("%-20s %-10d %-10.2f %-10s\n",
               list->students[i].name,
               list->students[i].roll,
               list->students[i].marks,
               (list->students[i].marks > 40) ? "Passed" : "Failed");
    }
}

void displayStudentConsole(const StudentList *list, int idx) {
    printf("Name: %s\n", list->students[idx].name);
    printf("Roll Number: %d\n", list->students[idx].roll);
    printf("Marks: %.2f\n", list->students[idx].marks);
    printf("Status: %s\n", (list->students[idx].marks > 40) ? "Passed" : "Failed");
}

void handleAddStudent(StudentList *list) {
    char name[NAME_LEN];
    int roll;
    float marks;
    
    printf("Enter student name: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Enter roll number: ");
    scanf("%d", &roll);
    getchar();

    printf("Enter marks: ");
    scanf("%f", &marks);
    getchar();

    if (addStudent(list, name, roll, marks)) {
        printf("Student record added successfully.\n");
        printf("%s has %s.\n", name, (marks > 40) ? "passed" : "failed");
    } else {
        printf("Error: A student with roll number %d already exists.\n", roll);
    }
}

void handleModifyStudent(StudentList *list) {
    int roll;
    printf("Enter roll number to modify: ");
    scanf("%d", &roll);
    getchar();
    
    int idx = searchStudent(list, roll); // Find index to show current name
    if (idx == -1) {
        printf("Student not found.\n");
        return;
    }

    printf("Modifying record for %s (Roll %d):\n", list->students[idx].name, list->students[idx].roll);
    
    char newName[NAME_LEN];
    printf("Enter new name (leave blank to keep current): ");
    fgets(newName, NAME_LEN, stdin);
    newName[strcspn(newName, "\n")] = 0;

    float newMarks;
    printf("Enter new marks (-1 to keep current): ");
    scanf("%f", &newMarks);
    getchar();

    if (modifyStudent(list, roll, newName, newMarks)) {
        printf("Record updated.\n");
    } else {
        printf("Error updating record (this shouldn't happen if student was found).\n");
    }
}

void handleRemoveStudent(StudentList *list) {
    int roll;
    printf("Enter roll number to remove: ");
    scanf("%d", &roll);
    getchar();
    
    if (removeStudent(list, roll)) {
        printf("Student record removed.\n");
    } else {
        printf("Student not found.\n");
    }
}

// --- The Main Function (The "Controller") ---

int main() {
    greetUser();

    StudentList list;
    initList(&list); // From student_logic.h

    int choice;
    do {
        printf("\n---- Student Record System ----\n");
        printf("1. Add student\n");
        printf("2. Display all students\n");
        printf("3. Modify student record\n");
        printf("4. Remove student record\n");
        printf("5. Search student by roll number\n");
        printf("6. Save records to file\n");
        printf("7. Load records from file\n");
        printf("8. Calculate average marks\n");
        printf("9. Sort records by marks\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); // consume newline

        switch (choice) {
            case 1:
                handleAddStudent(&list);
                break;
            case 2:
                displayStudentsConsole(&list);
                break;
            case 3:
                handleModifyStudent(&list);
                break;
            case 4:
                handleRemoveStudent(&list);
                break;
            case 5: {
                int roll;
                printf("Enter roll number to search: ");
                scanf("%d", &roll);
                int idx = searchStudent(&list, roll); // From student_logic.h
                if (idx != -1) {
                    printf("Student found:\n");
                    displayStudentConsole(&list, idx);
                } else {
                    printf("Student with roll number %d not found.\n", roll);
                }
                break;
            }
            case 6:
                if (saveToFile(&list)) { // From student_logic.h
                    printf("Records saved to file.\n");
                } else {
                    printf("Error saving file.\n");
                }
                break;
            case 7:
                if (loadFromFile(&list)) { // From student_logic.h
                    printf("Records loaded from file.\n");
                } else {
                    printf("Error opening file for reading.\n");
                }
                break;
            case 8: {
                float avg = getAverageMarks(&list); // From student_logic.h
                if (list.count > 0) {
                    printf("Average marks: %.2f\n", avg);
                } else {
                    printf("No student records.\n");
                }
                break;
            }
            case 9: {
                int order;
                printf("Enter 1 for ascending, 0 for descending: ");
                scanf("%d", &order);
                sortStudents(&list, order); // From student_logic.h
                printf("Records sorted by marks %s.\n", order ? "ascending" : "descending");
                break;
            }
            case 0:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 0);

    freeList(&list); // From student_logic.h
    return 0;
}