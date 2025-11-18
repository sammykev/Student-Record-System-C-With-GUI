#ifndef STUDENT_LOGIC_H
#define STUDENT_LOGIC_H

#include <stdio.h> // For FILE type

#define NAME_LEN 100
#define FILENAME "students.txt" // Using the original filename

// --- Struct Definitions ---

typedef struct {
    char name[NAME_LEN];
    int roll;
    float marks;
} Student;

typedef struct {
    Student *students;
    int count;
    int capacity;
} StudentList;

// --- Function Prototypes (The API) ---

// List management
void initList(StudentList *list);
void freeList(StudentList *list);
void ensureCapacity(StudentList *list); // This is internal, but GUI might need it

// Core data operations
int addStudent(StudentList *list, const char* name, int roll, float marks);
int removeStudent(StudentList *list, int roll);
int modifyStudent(StudentList *list, int roll, const char* newName, float newMarks);
int searchStudent(const StudentList *list, int roll); // This was already perfect

// Data processing
void sortStudents(StudentList *list, int ascending);
float getAverageMarks(const StudentList *list);

// File I/O
int saveToFile(const StudentList *list);
int loadFromFile(StudentList *list);

#endif // STUDENT_LOGIC_H