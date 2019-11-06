#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define isDigit(a) (a >= '0' && a <= '9')
#define isLetter(a) (a >= 'a' && a <= 'z' || a >= 'A' && a <= 'Z')
#define isValid(a) (isLetter(a) || isDigit(a))

#define GENERATING_CONST 53 /* a constant for generating function in hashing algorithm */
#define MODULE_CONST (const int) (1e9 + 9)
#define TABLE_SIZE 0x1000 /* 4096 */
#define BIG_PRIME_NUMBER 370801
#define A 168323L
#define B 211867L
#define SEED 74395212

#define MAX_COURSES 100
#define MAX_STUDENTS 1000

#define INPUT_FILE_NAME_SIZE 25

#define I_COURSES 0
#define I_PROFESSORS 1
#define I_TAS 2
#define I_STUDENTS 3

#define UNDEFINED 0
#define UNTRAINED_COURSE 1
#define LACKING_COURSE 2
#define TWO_COURSES 3

/*
 * Flags for scanning students.
 */
#define S_NAME 0
#define S_SURNAME 1
#define S_CODE 2
#define S_COURSES 3
#define STUDENT_CODE_SIZE 6

#define NAME 0
#define LABS 1
#define STUDENTS 2
#define END 3
#define BUFFER_SIZE 200

#define P_NAME 0
#define P_SURNAME 1
#define P_COURSES 2

#define POPULATION_SIZE 10000
#define KIDS_SIZE 1250
#define BEST_SIZE 50
#define MUTATION_SIZE 1200
#define GENERATIONS_NUMBER 0

#define MAX_BADNESS_POINTS 30000


/*
 * This functions is an implementation of polynomial hashing algorithm for strings.
 * This algorithm was chosen due to its low probability of collisions.
 * More information: https://en.wikipedia.org/wiki/Rolling_hash
 */
long long hash(const char *string) {
    long long value = 0;
    long long p = 1;

    while (*string != '\0') {
        value += p * (*string - 'A');

        p = p * GENERATING_CONST % MODULE_CONST;

        string++;
    }

    return value;
}

/*
 * This function is an implementation of MAD compression algorithm.
 * It is used to compress long long hash values into small values,
 * which will be used as array indexes in a simple hash table.
 */
int compressed_hash(char const *string) {
    long long hash_value = hash(string);
    int a = (int)((hash_value * A + B) % BIG_PRIME_NUMBER % TABLE_SIZE);
    if (a < 0) a = -a;
    return a;
}

/*
 * Returns maximum value of 2 integers.
 */
int maximum(int a, int b) {
    return a > b ? a : b;
}

/*
 * Compare two strings. If they are equal to each other: 0; otherwise: 1
 */
int compare_str(const char *a, const char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i])
            return 1;
        ++i;
    }

    if (a[i] == b[i]) return 0;
    return 1;
}


typedef struct professor_s {
    int id;
    char *name;
    int *courses;
} professor_t;

typedef struct ta_s {
    int id;
    char *name;
    int *courses;
} ta_t;

typedef struct student_s {
    int id;
    char *name;
    char *code;
    int *courses;
} student_t;

typedef struct course_s {
    int id;
    int hash_value;
    char *name;
    int labs_number;
    int students_number;
} course_t;

/*
 * Hashtable used for storing all courses.
 */
typedef struct courses_hashtable_s {
    course_t **courses;
} chash_t;

/*
 * Hashtable used for storing all professors.
 */
typedef struct professors_hashtable_s {
    professor_t **professors;
} phash_t;

/*
 * Hashtable used for storing all tas.
 */
typedef struct ta_hashtable_s {
    ta_t **tas;
} thash_t;


/*
 * Ta in course from individual.
 * Number - the number of labs ta is assigned to.
 */
typedef struct ta_in_course {
    int number;
    ta_t *ta;
}ta_c_t;

/*
 * Courses in individuals.
 */
typedef struct course_ind {
    short runnable;
    course_t *course;
    professor_t *prof; // assigned prof
    int ta_number;
    ta_c_t **tas;
} cind_t;

/*
 * Individual is used for algorithm.
 */
typedef struct individual {
    int badness_points;
    cind_t **cinds;
} ind_t;


/*
 * Default creator of professor
 */
professor_t *create_professor(int id, char *name, int *courses) {
    professor_t *professor = malloc(sizeof (professor_t));

    professor -> id = id;
    professor -> name = name;
    professor -> courses = courses;

    return professor;
}

/*
 * Print info about the given professor into standard output.
 */
void print_professor(professor_t *professor) {
    if (professor != NULL)
        printf("Professor {id: %d, full name: %s}\n", professor -> id, professor -> name);
    else
        printf("NULL Professor\n");
}

ta_t *create_ta(int id, char *name, int *courses) {
    ta_t *ta = malloc(sizeof (ta_t));

    ta -> id = id;
    ta -> name = name;
    ta -> courses = courses;

    return ta;
}

/*
 * Print info about the given TA into standard output.
 */
void print_ta(ta_t *ta) {
    if (ta != NULL)
        printf("TA {id: %d, full name: %s}\n", ta -> id, ta -> name);
    else
        printf("NULL TA\n");
}

course_t *create_course(int id, char *name, int labs_num, int students_num) {
    course_t *course = malloc(sizeof(course_t));

    course -> id = id;
    course -> hash_value = compressed_hash(name);
    course -> name = name;
    course -> labs_number = labs_num;
    course -> students_number = students_num;

    return course;
}

/*
 * Print info about the given course into standard output.
 */
void print_course(course_t *course) {
    if (course != NULL)
        printf("Course {id: %d, name: %s, labs: %d, students: %d}\n", course -> id, course -> name, course -> labs_number, course -> students_number);
    else printf("NULL Course\n");
}

/*
 * Create new student.
 */
student_t *create_student(int id, char *name, char *code, int *courses) {
    student_t *stud = malloc(sizeof(student_t));

    stud -> id = id;
    stud -> name = name;
    stud -> code = code;
    stud -> courses = courses;

    return stud;
}

/*
 * Creates new hash table of size TABLE_SIZE with NULL courses.
 */
chash_t *create_courses_hashtable() {
    chash_t *courses_hashtable = malloc(sizeof(courses_hashtable));
    courses_hashtable -> courses = malloc(TABLE_SIZE * sizeof(course_t*));

    for (int i = 0; i < TABLE_SIZE; ++i) {
        (courses_hashtable -> courses)[i] = NULL;
    }

    return courses_hashtable;
}

/*
 * Add new course to hashtable.
 */
int addCourseToHashTable(chash_t *courses_hashtable, course_t *course) {
    if (course == NULL) return 1;

    int i = compressed_hash(course -> name);

    int found = 1; // reverse!
    while ((courses_hashtable -> courses)[i] != NULL && (found = compare_str(course -> name, (courses_hashtable -> courses)[i] -> name))) { // TODO: create invalid input here for the same names
        i = (i + 1) % TABLE_SIZE;
    }
    if (!found)
        return 1;
    else
        courses_hashtable -> courses[i] = course;
    return 0;
}

/*
 * Get course by the name from the hashtable.
 */
course_t *getCourseFromHashTable(chash_t *c_hash, char const *name) {
    if (name == NULL) return NULL;

    int i = compressed_hash(name);
    int found = 1;

    /*
     * For loop that finds index i where the course is stored.
     * Terminates when it meets NULL pointer or course with the same name (it means course is found).
     */
    for (; (c_hash -> courses)[i] != NULL && (found = compare_str((c_hash -> courses[i]) -> name, name)); i = (i + 1) % TABLE_SIZE);

    return !found ? c_hash -> courses[i] : NULL;
}

/*
 * Get id of course by the given name.
 */
int getCourseIdFromHashTable(chash_t *c_hash, char const *name) {
    course_t *course = getCourseFromHashTable(c_hash, name);

    if (course == NULL) return -1;
    return course -> id;
}

/*
 * Creates new hash table of size TABLE_SIZE with NULL professors.
 */
phash_t *create_profs_hashtable() {
    phash_t *profs_hashtable = malloc(sizeof(profs_hashtable));
    profs_hashtable -> professors = malloc(TABLE_SIZE * sizeof(professor_t*));

    for (int i = 0; i < TABLE_SIZE; ++i) {
        (profs_hashtable -> professors)[i] = NULL;
    }

    return profs_hashtable;
}

/*
 * Add a professor to hashtable.
 */
int addProfToHashTable(phash_t *profs_hashtable, professor_t *professor) {
    if (professor == NULL) return 1;

    int i = compressed_hash(professor -> name);

    int found = 1; // reverse!
    while ((profs_hashtable -> professors)[i] != NULL && (found = compare_str(professor -> name, (profs_hashtable -> professors)[i] -> name))) { // TODO: create invalid input here for the same names
        i = (i + 1) % TABLE_SIZE;
    }
    if (!found)
        return 1;
    else
        (profs_hashtable -> professors)[i] = professor;
    return 0;
}

/*
 * Get a professor from hashtable by the name.
 */
professor_t *getProfFromHashTable(phash_t *p_hash, char const *name) {
    if (name == NULL) return NULL;

    int i = compressed_hash(name);
    int found = 1;

    /*
     * For loop that finds index i where the professor is stored.
     * Terminates when it meets NULL pointer or a professor with the same name (it means the professor is found).
     */
    for (; (p_hash -> professors)[i] != NULL && (found = compare_str((p_hash -> professors[i]) -> name, name)); i = (i + 1) % TABLE_SIZE);

    return !found ? p_hash -> professors[i] : NULL;
}

/*
 * Creates new hash table of size TABLE_SIZE with NULL professors.
 */
thash_t *create_tas_hashtable() {
    thash_t *tas_hashtable = malloc(sizeof(tas_hashtable));
    tas_hashtable -> tas = malloc(TABLE_SIZE * sizeof(professor_t*));

    for (int i = 0; i < TABLE_SIZE; ++i) {
        (tas_hashtable -> tas)[i] = NULL;
    }

    return tas_hashtable;
}

/*
 * Add a TA to hashtable.
 */
int addTaToHashTable(thash_t *tas_hashtable, ta_t *ta) {
    if (ta == NULL) return 1;

    int i = compressed_hash(ta -> name);

    int found = 1; // reverse!
    while ((tas_hashtable -> tas)[i] != NULL && (found = compare_str(ta -> name, (tas_hashtable -> tas)[i] -> name))) { // TODO: create invalid input here for the same names
        i = (i + 1) % TABLE_SIZE;
    }
    if (!found)
        return 1;
    else
        (tas_hashtable -> tas)[i] = ta;
    return 0;
}

/*
 * Get TA from hashtable.
 */
ta_t *getTaFromHashTable(thash_t *t_hash, char const *name) {
    if (name == NULL) return NULL;

    int i = compressed_hash(name);
    int found = 1;

    /*
     * For loop that finds index i where the professor is stored.
     * Terminates when it meets NULL pointer or a professor with the same name (it means the professor is found).
     */
    for (; (t_hash -> tas)[i] != NULL && (found = compare_str((t_hash -> tas[i]) -> name, name)); i = (i + 1) % TABLE_SIZE);

    return !found ? t_hash -> tas[i] : NULL;
}

/*
 * Create pool of tas.
 * tas_pool[i] = array of tas id, who can be assigned to course i.
 */
int **create_tas_pool(int C, int T, ta_t **tas) {
    int **ta_pool = malloc(C * sizeof(int *));
    for (int i = 0; i < C; ++i) {
        ta_pool[i] = malloc((T + 1) * sizeof(int)); // first element is for size
        ta_pool[i][0] = 0;
    }

    for (int i = 0; i < T; ++i) {
        for (int j = 1; j < tas[i] -> courses[0] + 1; ++j) {
            ta_pool[tas[i] -> courses[j]][++ta_pool[tas[i] -> courses[j]][0]] = i;
        }
    }

    return ta_pool;
}

/*
 * Create c_studs.
 * c_studs[i] = the number of students who want to enroll in course i.
 */
int *create_c_studs(int C, int S, student_t **studs) {
    int *c_studs = malloc(C * sizeof(int)); // free this
    memset(c_studs, 0, C * sizeof(int));

    for (int i = 0; i < S; ++i) {
        for (int j = 1; j < studs[i] -> courses[0] + 1; ++j) {
            c_studs[studs[i] -> courses[j]] += 1;
        }
    }

    return c_studs;
}


/*
 * Generate random int between start and end
 */
int randInt(int start, int end) {
    return (rand() % (end - start)) + start;
}

/*
 * Generate random array that contains all numbers of interval [a; b)
 */
int *create_shuffle(int start, int end) {
    int size = end - start;
    if (size < 0) return NULL;
    int *shuffled = malloc(sizeof(int) * size);
    for (int i = start; i < end; ++i) {
        shuffled[i - start] = i;
    }

    for (int i = 0; i < size; ++i) {
        int j = randInt(0, size);
        if (j == i) continue;
        shuffled[i] += shuffled[j];
        shuffled[j] = shuffled[i] - shuffled[j];
        shuffled[i] = shuffled[i] - shuffled[j];
    }

    return shuffled;
}

typedef struct p_ind {
    course_t *course1;
    course_t *course2;
} p_ind_t;

/*
 * Check if the professor has the given course (yes - 1; no - 0).
 */
int prof_has_course(professor_t *prof, course_t *course) {
    if (prof == NULL || course == NULL) return 0;

    for (int i = 1; i < 1 + prof -> courses[0]; ++i) {
        if (prof -> courses[i] == course -> id) return 1;
    }

    return 0;
}

/*
 * Create course inside individual.
 */
cind_t *create_cind(course_t *course) {
    cind_t *cind = malloc(sizeof(cind_t));

    cind -> runnable = 0;
    cind -> course = course;
    cind -> prof = NULL;
    cind -> ta_number = 0;
    cind -> tas = NULL;

    return cind;
}

/*
 * Add TA to course inside individual.
 */
void add_ta_to_cind(cind_t *cind, ta_t *ta, int num) {
    cind -> tas[cind -> ta_number] = malloc(sizeof(ta_c_t)); // free here
    cind -> tas[cind -> ta_number] -> number = num;
    cind -> tas[cind -> ta_number] -> ta = ta;
    cind -> ta_number++;
}

/*
 * Randomly distribute professors in a given individual.
 */
void distr_profs(int P, professor_t **profs, int C, course_t **courses, ind_t *ind) {
    int *shuffled = create_shuffle(0, C);

    int cur_course = 0;

    for (int i = 0; i < P; ++i) {
        if (cur_course < C) {
            ind -> cinds[shuffled[cur_course]] -> prof = profs[i];
            if (cur_course + 1 < C && prof_has_course(profs[i], courses[shuffled[cur_course]]) && prof_has_course(profs[i], courses[shuffled[cur_course + 1]])) {
                ind -> cinds[shuffled[cur_course + 1]] -> prof = profs[i];
                ++cur_course;
            }

            ++cur_course;
        }
    }
    free(shuffled);
}

/*
 * Randomly distribute TAs in a given individual.
 */
void distr_tas(int C, int T, course_t **courses, ta_t **tas, int **tas_pool, ind_t *ind) {
    int *avail_tas = malloc(T * sizeof(int)); // array of availability status
    for (int i = 0; i < T; ++i) {
        avail_tas[i] = 4;
    }

    for (int i = 0; i < C; ++i) {
        int *shuffled = create_shuffle(1, tas_pool[i][0] + 1);
        int tas_needed = courses[i] -> labs_number;
        int wentAll = tas_pool[i][0] == 0;
        int curTA = 0;

        if (ind -> cinds[i] -> prof == NULL) continue;

        ind -> cinds[i] -> tas = malloc((MAX_COURSES + 1) * sizeof(ta_c_t*)); // free here
        while (tas_needed > 0 && !wentAll) {
            if (avail_tas[tas_pool[i][shuffled[curTA]]] != 0 && tas_needed <= avail_tas[tas_pool[i][shuffled[curTA]]]) {
                add_ta_to_cind(ind -> cinds[i], tas[tas_pool[i][shuffled[curTA]]], tas_needed);

                avail_tas[tas_pool[i][shuffled[curTA]]] -= tas_needed;
                tas_needed = 0;
            } else if (avail_tas[tas_pool[i][shuffled[curTA]]] != 0) {
                add_ta_to_cind(ind -> cinds[i], tas[tas_pool[i][shuffled[curTA]]], avail_tas[tas_pool[i][shuffled[curTA]]]);

                tas_needed -= avail_tas[tas_pool[i][shuffled[curTA]]];
                avail_tas[tas_pool[i][shuffled[curTA]]] = 0;
            }

            ++curTA;
            wentAll = tas_pool[i][0] == curTA;
        }

        if (tas_needed > 0) {
            for (int j = 0; j < ind -> cinds[i] -> ta_number; ++j) {
                avail_tas[ind -> cinds[i] -> tas[j] -> ta -> id] = ind -> cinds[i] -> tas[j] -> number; // sos
                free(ind -> cinds[i] -> tas[j]);
            }

            free(ind -> cinds[i] -> tas);

            ind -> cinds[i] -> ta_number = 0;
            ind -> cinds[i] -> tas = NULL;
            ind -> cinds[i] -> prof = NULL;
            ind -> cinds[i] -> runnable = 0;
        } else {
            ind -> cinds[i] -> runnable = 1;
        }

        free(shuffled);
    }
}

/*
 * Free space that was used by individual structure.
 */
void free_ind(int C, ind_t *ind) {
    for (int i = 0; i < C; ++i) {
        for (int j = 0; j < ind -> cinds[i] -> ta_number; ++j) {
            free(ind -> cinds[i] -> tas[j]);
        }

        free(ind -> cinds[i]);
    }

    free(ind);
}

/*
 * Calculate badness points for a given individual.
 * Give MAX_BADNESS_POINTS to individuals that cannot exist.
 */
int calculate_badness(int C, int P, int T, course_t **courses, ind_t *ind, int *c_studs) {
    int badness_points = 0;
    int *profs_badness = malloc(P * sizeof(int));
    int *tas_badness = malloc(T * sizeof(int));

    memset(profs_badness, 0, (size_t) P * sizeof(int));
    memset(tas_badness, 0, (size_t) T * sizeof(int));


    for (int i = 0; i < C; ++i) {
        if (!ind->cinds[i] -> runnable) {
            badness_points += 20;
            badness_points += c_studs[i];
        } else {
            profs_badness[ind->cinds[i] -> prof -> id]++;
            badness_points += maximum(0, c_studs[i] - ind->cinds[i]->course->students_number);
            for (int j = 0; j < ind -> cinds[i] -> ta_number; ++j) {
                tas_badness[ind -> cinds[i] -> tas[j] -> ta -> id] += ind -> cinds[i] -> tas[j] -> number;
            }
        }
    }

    for (int i = 0; i < P; ++i) {
        if (2 - profs_badness[i] < 0) {
            ind -> badness_points = MAX_BADNESS_POINTS;
            break;
        }
        badness_points += 5 * (2 - profs_badness[i]);
    }

    for (int i = 0; i < T; ++i) {
        if ((ind -> badness_points == MAX_BADNESS_POINTS) || 4 - tas_badness[i] < 0) {
            ind -> badness_points = MAX_BADNESS_POINTS;
            break;
        }
        badness_points += 2 * (4 - tas_badness[i]);
    }

    free(profs_badness);
    free(tas_badness);

    ind -> badness_points = badness_points;
    return badness_points;
}

/*
 * Choose BEST_SIZE best individuals in population inds.
 */
void choose_best_inds(int C, ind_t **inds) {
    int *was = malloc(POPULATION_SIZE * sizeof(int)); // free here
    ind_t **best_inds = malloc(BEST_SIZE * sizeof(ind_t *));
    memset(was, 0, POPULATION_SIZE * sizeof(int));

    for (int i = 0; i < BEST_SIZE; ++i) {
        int cur_best = MAX_BADNESS_POINTS;
        int cur_best_i = -1;
        for (int j = 0; j < POPULATION_SIZE; ++j) {
            if (!was[j] && inds[j] -> badness_points < cur_best) {
                cur_best = inds[j] -> badness_points;
                cur_best_i = j;
            }
        }
        best_inds[i] = inds[cur_best_i];
        was[cur_best_i] = 1;
    }

    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (!was[i]) {
            free_ind(C, inds[i]);
        }
        inds[i] = i < BEST_SIZE ? best_inds[i] : NULL;
    }

    free(best_inds);
    free(was);
}

/*
 * Create a random individual.
 */
ind_t *create_ind(int C, int P, int T, course_t **courses, professor_t **profs, ta_t **tas, int **tas_pool, int *c_studs) {
    ind_t *ind = malloc(sizeof(ind_t)); // free here
    ind -> cinds = malloc(C * sizeof(cind_t*)); // free here

    for (int i = 0; i < C; ++i) {
        ind -> cinds[i] = create_cind(courses[i]); // free here
    }

    distr_profs(P, profs, C, courses, ind);
    distr_tas(C, T, courses, tas, tas_pool, ind);

    calculate_badness(C, P, T, courses, ind, c_studs);

    return ind;
}

/*
 * Struct that is used for returning information about token in nextToken function.
 */
struct flag_s {
    short contains_digits: 1;
    short contains_letters: 1;
    short last_token: 1;
    short contains_invalid_symbs: 1;
    int length;
};

/*
 * Set flag's fields to default values.
 */
void clearFlag(struct flag_s *flag) {
    flag -> contains_digits = 0;
    flag -> contains_letters = 0;
    flag -> last_token = 0;
    flag -> contains_invalid_symbs = 0;
    flag -> length = 0;
}

/*
 * Put a part of given line until a space to buffer.
 * Also put flags to flag structure.
 */
char *nextToken(char* buf, size_t size, char *line, struct flag_s *flag) {
    int i = 0;
    for (i = 0; i < size - 1 && line[i] != '\0' && line[i] != ' ' && line[i] != '\n'; ++i) {
        buf[i] = line[i];
        flag -> contains_digits = flag -> contains_digits || isDigit(line[i]);
        flag -> contains_letters = flag -> contains_letters || isLetter(line[i]);
        flag -> contains_invalid_symbs = flag -> contains_invalid_symbs || !isValid(line[i]);
    }

    buf[i] = '\0';
    flag -> length = i;
    flag -> last_token = line[i] == '\0' || line[i] == '\n';
    if ((buf[0] == 'P' || buf[0] == 'T' || buf[0] == 'S') && buf[1] == '\0') flag -> contains_invalid_symbs = 1;
    return line + i + 1;
}

/*
 * Convert string to integer.
 * If any error -> return -1.
 */
int strtint(const char str[]) {
    int i = 0;
    int num = 0;
    while (str[i]) {
        if (i == 1 && num == 0)
            return -1;
        num = num * 10 + str[i] - '0';
        i++;
    }

    return i != 0 && num != 0 ? num : -1;
}

/*
 * Get course from string.
 */
course_t *get_c_line(int id, char *line, chash_t *chash) {
    char *name = malloc(BUFFER_SIZE);
    int labs_number = 0, students_number = 0;
    int error = 0, last_token = 0;
    char *buffer = malloc(BUFFER_SIZE);
    struct flag_s *flag = malloc(sizeof(struct flag_s));
    clearFlag(flag);
    int state = NAME;

    while (!last_token) {
        line = nextToken(buffer, BUFFER_SIZE, line, flag);
        if (state == NAME) {
            if (flag -> contains_digits || flag -> contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(name, buffer);
        } else if (state == LABS) {
            if (flag -> contains_letters || flag -> contains_invalid_symbs || (labs_number = strtint(buffer)) == -1) {
                error = 1;
                break;
            }
        } else if (state == STUDENTS) {
            if (flag -> contains_letters || flag -> contains_invalid_symbs || (students_number = strtint(buffer)) == -1) {
                error = 1;
                break;
            }
        } else if (state == END) {
            error = 1;
            break;
        }
        last_token = flag -> last_token;
        clearFlag(flag);
        state++;
    }

    course_t *course = create_course(id, name, labs_number, students_number);
    if (addCourseToHashTable(chash, course)) error = 1;

    free(buffer);
    free(flag);

    if (state != END || error) {
        free(name);
        free(course);
        return NULL;
    }

    return course;
}

/*
 * Get professor from string.
 */
professor_t *get_p_line(int id, char *line, chash_t *chash, phash_t *phash) {
    int statesShifts[] = {P_SURNAME, P_COURSES, P_COURSES};

    char *name = malloc(BUFFER_SIZE);
    char *surname = name;
    int *courses = malloc(sizeof(int) * (MAX_COURSES + 1));
    courses[0] = 0; // 0-th <- number of courses

    int state = P_NAME;
    char *buffer = malloc(BUFFER_SIZE);
    int error = 0, last_token = 0;
    struct flag_s *flag = malloc(sizeof(struct flag_s));
    clearFlag(flag);

    while (!last_token) {
        line = nextToken(buffer, BUFFER_SIZE, line, flag);
        if (state == P_NAME) {
            if (flag -> contains_digits || flag -> contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(name, buffer);
            name[flag -> length] = ' ';
            surname = name + flag -> length + 1;
        } else if (state == P_SURNAME) {
            if (flag -> contains_digits || flag -> contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(surname, buffer);
        } else if (state == P_COURSES) {
            int courseId;
            if (flag -> contains_digits || flag -> contains_invalid_symbs || (courseId = getCourseIdFromHashTable(chash, buffer)) == -1) {
                error = 1;
                break;
            }

            courses[++courses[0]] = courseId;
        }

        last_token = flag -> last_token;
        clearFlag(flag);
        state = statesShifts[state];
    }

    free(flag);
    free(buffer);

    professor_t *professor = create_professor(id, name, courses);

    if (addProfToHashTable(phash, professor)) error = 1;


    if (courses[0] == 0 || state != P_COURSES || error) {
        free(name);
        free(courses);
        free(professor);
        return NULL;
    }

    return professor;
}

/*
 * Get TA from string.
 */
ta_t *get_t_line(int id, char *line, chash_t *chash, thash_t *thash) {
    int statesShifts[] = {P_SURNAME, P_COURSES, P_COURSES};

    char *name = malloc(BUFFER_SIZE);
    char *surname = name;
    int *courses = malloc(sizeof(int) * (MAX_COURSES + 1));
    courses[0] = 0; // 0-th <- number of courses

    int state = P_NAME;
    char *buffer = malloc(BUFFER_SIZE);
    int error = 0, last_token = 0;
    struct flag_s *flag = malloc(sizeof(struct flag_s));
    clearFlag(flag);

    while (!last_token) {
        line = nextToken(buffer, BUFFER_SIZE, line, flag);
        if (state == P_NAME) {
            if (flag -> contains_digits || flag -> contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(name, buffer);
            name[flag -> length] = ' ';
            surname = name + flag -> length + 1;
        } else if (state == P_SURNAME) {
            if (flag -> contains_digits || flag -> contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(surname, buffer);
        } else if (state == P_COURSES) {
            int courseId;
            if (flag -> contains_digits || flag -> contains_invalid_symbs || (courseId = getCourseIdFromHashTable(chash, buffer)) == -1) {
                error = 1;
                break;
            }

            courses[++courses[0]] = courseId;
        }

        last_token = flag -> last_token;
        clearFlag(flag);
        state = statesShifts[state];
    }

    free(flag);
    free(buffer);

    ta_t *ta = create_ta(id, name, courses);

    if (addTaToHashTable(thash, ta)) error = 1;


    if (courses[0] == 0 || state != P_COURSES || error) {
        free(name);
        free(courses);
        free(ta);
        return NULL;
    }

    return ta;
}

/*
 * Check whether the code is unique.
 */
int find_s_code(int S, char *code, student_t **studs) {
    for (int i = 0; i < S; ++i) {
        if (!compare_str(code, studs[i] -> code)) return 1;
    }
    return 0;
}

/*
 * Get student from string.
 */
student_t *get_s_line(int id, char *line, student_t **studs, chash_t *chash) {
    int statesShifts[] = {S_SURNAME, S_CODE, S_COURSES, S_COURSES};

    char *name = malloc(BUFFER_SIZE);
    char *surname = name;
    char *code = malloc(STUDENT_CODE_SIZE);
    int *courses = malloc(sizeof(int) * (MAX_COURSES + 1));
    courses[0] = 0; // 0-th <- number of courses

    int state = S_NAME; // used for smart error handling
    char *buffer = malloc(BUFFER_SIZE); // buffer for scanning
    int error = 0, last_token = 0;
    struct flag_s *flag = malloc(sizeof(struct flag_s));
    clearFlag(flag);

    while (!last_token) {
        line = nextToken(buffer, BUFFER_SIZE, line, flag);
        if (state == S_NAME) {
            if (flag->contains_digits || flag->contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(name, buffer);
            name[flag->length] = ' ';
            surname = name + flag->length + 1; // surname is shifted name, used for merging name and surname
        } else if (state == S_SURNAME) {
            if (flag->contains_digits || flag->contains_invalid_symbs) {
                error = 1;
                break;
            }
            strcpy(surname, buffer);
        } else if (state == S_CODE) {
            if (flag->length != 5 || flag->contains_invalid_symbs) {
                error = 1;
                break;
            }

            strcpy(code, buffer);
        } else if (state == S_COURSES) {
            int courseId;
            if (flag->contains_digits || flag->contains_invalid_symbs ||
                (courseId = getCourseIdFromHashTable(chash, buffer)) == -1) {
                error = 1;
                break;
            }

            courses[++courses[0]] = courseId;
        }

        last_token = flag->last_token;
        clearFlag(flag);
        state = statesShifts[state];
    }

    student_t *stud = create_student(id, name, code, courses);
    if (find_s_code(id, code, studs)) error = 1;

    free(flag);
    free(buffer);

    // if 0 courses or we did not reach courses or some error
    if (courses[0] == 0 || state != S_COURSES || error) {
        free(name);
        free(courses);
        free(stud);
        return NULL;

    }

    return stud;
}

/*
 * Generate first (zero) population.
 */
ind_t **generate_population_zero(int C, int P, int T, course_t **courses, professor_t **profs, ta_t **tas, int **tas_pool, int *c_studs) {
    ind_t **pop0 = malloc(POPULATION_SIZE * sizeof(ind_t *)); // free here
    for (int j = 0; j < POPULATION_SIZE; ++j) {
        pop0[j] = create_ind(C, P, T, courses, profs, tas, tas_pool, c_studs); // create random individual
    }
    return pop0;
}

ind_t *get_best_sol(int C, int P, int T, course_t **courses, professor_t **profs, ta_t **tas, int **tas_pool, int *c_studs) {
    ind_t **cur_pop = generate_population_zero(C, P, T, courses, profs, tas, tas_pool, c_studs);

    for (int i = 0; i < GENERATIONS_NUMBER; ++i) {
        choose_best_inds(C, cur_pop);
    }
    choose_best_inds(C, cur_pop);
    for (int i = 1; i < BEST_SIZE; ++i) {
        free_ind(C, cur_pop[i]);
    }

    ind_t *best = cur_pop[0];
    free(cur_pop);
    return best;
}

/*
 * Is a in arr. arr[0] = size of arr;
 */
int is_in_courses(int a, const int *arr) {
    for (int i = 1; i < arr[0] + 1; ++i) {
        if (arr[i] == a) return 1;
    }
    return 0;
}

/*
 * Print final version to existing output file.
 */
void format_ind(int C, int P, int T, int S, course_t **courses, professor_t **profs, ta_t **tas, student_t **studs, ind_t *ind, FILE *out) {
    if (out == NULL) return;

    int *courses_places = malloc(C * sizeof(int)); // how many places exist for each course
    int *profs_flags = malloc(P * sizeof(int)); // flags for professors
    int *profs_un_c = malloc(P * sizeof(int)); // used for storing untrained course
    memset(profs_flags, 0, P * sizeof(int));
    memset(profs_un_c, 0, P * sizeof(int));

    int *tas_busy = malloc(T * sizeof(int)); // how busy tas are
    memset(tas_busy, 0, T * sizeof(int));

    for (int i = 0; i < C; ++i) {
        if (ind -> cinds[i] -> runnable) {
            courses_places[i] = ind -> cinds[i] -> course -> students_number;

            fprintf(out, "%s\n%s\n", ind -> cinds[i] -> course -> name, ind -> cinds[i] -> prof -> name);
            if (!prof_has_course(ind -> cinds[i] -> prof, ind -> cinds[i] -> course)) { // if course is untrained
                profs_flags[ind -> cinds[i] -> prof -> id] = UNTRAINED_COURSE;
                profs_un_c[ind -> cinds[i] -> prof -> id] = ind -> cinds[i] -> course -> id;
            } else {

                if (profs_flags[ind -> cinds[i] -> prof -> id] == UNDEFINED) { // if 0 courses
                    profs_flags[ind -> cinds[i] -> prof -> id] = LACKING_COURSE; // then lacking
                }
                else if (profs_flags[ind -> cinds[i] -> prof -> id] == LACKING_COURSE)
                    profs_flags[ind -> cinds[i] -> prof -> id] = TWO_COURSES; // 2 courses, no problems

            }

            for (int j = 0; j < ind -> cinds[i] -> ta_number; ++j) {
                tas_busy[ind -> cinds[i] -> tas[j] -> ta -> id] += ind -> cinds[i] -> tas[j] -> number;
                for (int k = 0; k < ind -> cinds[i] -> tas[j] -> number; ++k) {
                    fprintf(out, "%s\n", ind -> cinds[i] -> tas[j] -> ta -> name);
                }
            }

            int st_num = 0;
            for (int j = 0; j < S; ++j) {
                if (is_in_courses(i, studs[j]->courses)) {
                    ++st_num;
                    fprintf(out, "%s %s\n", studs[j] -> name, studs[j] -> code);
                    if (st_num == courses_places[i])
                        break;
                }
            }

            fprintf(out, "\n");
        } else {
            courses_places[i] = -1;
        }
    }

    for (int i = 0; i < C; ++i) {
        if (courses_places[i] == -1) {
            fprintf(out, "%s cannot be run.\n", courses[i] -> name);
            courses_places[i] = 0;
        }
    }

    for (int i = 0; i < P; ++i) {
        if (profs_flags[i] == UNDEFINED) {
            fprintf(out, "%s is unassigned.\n", profs[i] -> name);
        }
    }

    for (int i = 0; i < P; ++i) {
        if (profs_flags[i] == UNTRAINED_COURSE) {
            fprintf(out, "%s is not trained for %s.\n", profs[i] -> name, courses[profs_un_c[i]]->name);
        } else if (profs_flags[i] == LACKING_COURSE) {
            fprintf(out, "%s is lacking class.\n", profs[i] -> name);
        }
    }

    for (int i = 0; i < T; ++i) {
        if (4 - tas_busy[i] > 0) {
            fprintf(out, "%s is lacking %d lab(s).\n", tas[i] -> name, 4 - tas_busy[i]);
        }
    }

    for (int i = 0; i < S; ++i) {
        for (int j = 1; j < studs[i] -> courses[0] + 1; ++j) {
            if (courses_places[studs[i] -> courses[j]] == 0) {
                fprintf(out, "%s is lacking %s.\n", studs[i] -> name, courses[studs[i] -> courses[j]] -> name);
            } else {
                courses_places[studs[i] -> courses[j]]--;
            }
        }
    }
    fprintf(out, "Total score is %d.", ind -> badness_points);


    // free everything
    free(courses_places);
    free(profs_flags);
    free(profs_un_c);
    free(tas_busy);
}

/*
 * Print error in existing output file.
 */
void print_error(FILE *file) {
    fprintf(file, "Invalid input.");
}

/*
 * Solve task for given existing file input and output.
 */
void solve(FILE *input, FILE *output) {
    srand(SEED);

    int C = 0, P = 0, T = 0, S = 0;
    course_t **courses = malloc(MAX_COURSES * sizeof(course_t *));
    professor_t **profs = malloc(MAX_COURSES * sizeof(professor_t *));
    ta_t **tas = malloc(MAX_COURSES * sizeof(ta_t *));
    student_t **studs = malloc(MAX_STUDENTS * sizeof(student_t *));

    int *c_studs = NULL;

    chash_t *chash = create_courses_hashtable();
    phash_t *phash = create_profs_hashtable();
    thash_t *thash = create_tas_hashtable();

    int **tas_pool = NULL;

    int wait[] = {'P', 'T', 'S', 256};
    int state = I_COURSES;

    char *line = malloc(500);
    int error = 0;
    fgets(line, 500, input);
    while (1) {
        if (line[0] == wait[state]) {
            if (line[1] == '\n' || line[1] == '\0') {
                state++;
                if (feof(input)) break;
                fgets(line, 500, input);
                continue;
            }
        }

        if (state == I_COURSES) {
            course_t *course = get_c_line(C, line, chash);
            if (course == NULL) {
                error = 1;
                break;
            }
            courses[C++] = course;
        } else if (state == I_PROFESSORS) {
            professor_t *professor = get_p_line(P, line, chash, phash);
            if (professor == NULL) {
                error = 1;
                break;
            }
            profs[P++] = professor;
        } else if (state == I_TAS) {
            ta_t *ta = get_t_line(T, line, chash, thash);
            if (ta == NULL) {
                error = 1;
                break;
            }

            tas[T++] = ta;
        } else if (state == I_STUDENTS) {
            student_t *student = get_s_line(S, line, studs, chash);
            if (student == NULL) {
                error = 1;
                break;
            }

            studs[S++] = student;
        }

        if (feof(input)) break;
        fgets(line, 500, input);
    }

    if (state != I_STUDENTS || error) {
        print_error(output);
    } else {
        tas_pool = create_tas_pool(C, T, tas);
        c_studs = create_c_studs(C, S, studs);

        ind_t *sol = get_best_sol(C, P, T, courses, profs, tas, tas_pool, c_studs);
        format_ind(C, P, T, S, courses, profs, tas, studs, sol, output);
    }


    for (int i = 0; i < C; ++i) {
        free(courses[i]->name);
        free(courses[i]);
    }
    for (int i = 0; i < P; ++i) {
        free(profs[i]->name);
        free(profs[i]->courses);
        free(profs[i]);
    }
    for (int i = 0; i < T; ++i) {
        free(tas[i]->courses);
        free(tas[i]->name);
        free(tas[i]);
    }
    for (int i = 0; i < S; ++i) {
        free(studs[i]->name);
        free(studs[i]->courses);
        free(studs[i]->code);
        free(studs[i]);
    }

    free(line);

    if (c_studs != NULL)
        free(c_studs);

    if (tas_pool != NULL) {
        for (int i = 0; i < C; ++i) {
            free(tas_pool[i]);
        }

        free(tas_pool);
    }

    free(chash->courses);
    free(chash);

    free(phash->professors);
    free(phash);

    free(thash->tas);
    free(thash);
}

/*
 * Scan all files from input50.txt to input1.txt and solve task for existing files.
 */
void scan_files() {
    char input_name[INPUT_FILE_NAME_SIZE];
    char output_name[INPUT_FILE_NAME_SIZE];
    int file_found = 0;
    for (int i = 50; i >= 1; i--) {
        sprintf(input_name, "input%d.txt", i);
        sprintf(output_name, "ArtemBahanovOutput%d.txt", i);

        FILE *input = fopen(input_name, "r");

        if (input == NULL) {
            if (!file_found) continue;
            else {
                FILE *output = fopen(output_name, "w");
                print_error(output);
                fclose(output);
            }
        } else {
            file_found = 1;
            FILE *output = fopen(output_name, "w");
            solve(input, output);
            fclose(output);
            fclose(input);
        }
    }
}

int main() {

    FILE *email_file = fopen("ArtemBahanovEmail.txt", "w");
    fprintf(email_file, "a.bahanov@innopolis.university");
    fclose(email_file);

    scan_files();

}