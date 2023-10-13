#ifndef COURSE_RECORD
#define COURSE_RECORD

struct Course
{
    int course_id; // 0, 1, 2 ....
    char course_name[25]; // Course Name
    int seat; // Total number on seat
    int alloted_seat; // Total number of filled seat
};

#endif