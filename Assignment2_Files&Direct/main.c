#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<time.h>

#define PREFIX "movies_"

struct movies;
struct movies *createMovies(char* currline);
struct movies* processFile(char* filename);
void largest_finding();
void smallest_finding();
void create_dir(char* fileName);
void create_file(struct movies* list, char* dirName);



struct movies {
    char* title;
    char* year;
    char* languages;
    char* rate;
    struct movies* next;
};

struct movies *createMovies(char* currline) {
    struct movies* currMovie = malloc(sizeof(struct movies));
    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currline, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // The next token is the year
    token = strtok_r(NULL, ", ", &saveptr);
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token);

    // The next token is the languages
    token = strtok_r(NULL, ", ", &saveptr);
    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->languages, token);

    // The last token is the rate
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rate = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->rate, token);

    // Set the next node to NULL in the newly created movie entry
    currMovie->next = NULL;

    return currMovie;
}

struct movies* processFile(char* filename) {
    FILE *fptr = fopen(filename, "r");
    
    char *currline = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    struct movies *head = NULL;
    struct movies *tail = NULL;

    getline(&currline, &len, fptr);
    while ((nread = getline(&currline, &len, fptr)) != -1)
    {
        // printf("%s", currline);
        // Get a new movie node corresponding to the current line
        struct movies *newNode = createMovies(currline);
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }
    free(currline);
    fclose(fptr);
    return head;
}

void create_file(struct movies* list, char* dirName) {    
    // create a new file in that correct path
    while(list != NULL) {
        char file_path[100];
        sprintf(file_path, "%s/%s%s", dirName, list->year, ".txt");
        // printf("\n\n%s\n", file_path);
        FILE* fptr = fopen(file_path, "a");
        fprintf(fptr, "%s\n", list->title);
        chmod(file_path, 0640);
        fclose(fptr);
        list = list->next;
    }
}

void create_dir(char* fileName) {
    // Create a new directory with a name.
    srand(time(NULL));
    int r = rand() % 100000;
    char* dir_name = malloc(sizeof(char) * 25);
    sprintf(dir_name, "./yangca.movie.%d", r);
    mkdir(dir_name, 0750);
    printf("Created directory with name: yangca.movies.%d\n", r); 
    struct movies *list = processFile(fileName);
    create_file(list, dir_name);
}

void largest_finding() {
    // This initialization is what I learned from Exploration: Directories in Module 3.
    DIR *currDir = opendir("./");
    struct dirent *aDir;
    struct stat dirStat;
    int max_size = 0;
    char* max_name;

    while ((aDir = readdir(currDir)) != NULL) {
        // loop through all dir entries and find the max size dir entry.
        if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strcmp(strstr(aDir->d_name, ".csv"), ".csv") == 0) {      
            stat(aDir->d_name, &dirStat);
            if (dirStat.st_size > max_size) {
                max_size = dirStat.st_size;
                max_name = aDir->d_name;
            }
        }
    }
    printf("\nNow processing the chosen file named %s has the maximum size: %d \n", max_name, max_size);
    // Next we need to process the file: create a dir, and create some files under that dir.
    create_dir(max_name);
    closedir(currDir);
}

void smallest_finding() {
    // This initialization is what I learned from Exploration: Directories in Module 3.
    DIR *currDir = opendir("./");
    struct dirent *aDir;
    struct stat dirStat;
    int min_size = 999999;
    char* min_name;

    while ((aDir = readdir(currDir)) != NULL) {
        // loop through all dir and find the max size dir.
        if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strcmp(strstr(aDir->d_name, ".csv"), ".csv") == 0) {      
            stat(aDir->d_name, &dirStat);
            if (dirStat.st_size < min_size) {
                min_size = dirStat.st_size;
                min_name = aDir->d_name;
            }
        }
    }
    printf("\nNow processing the chosen file named %s has the minimum size: %d \n", min_name, min_size);
    // Next we need to process the file: create a dir, and create some files under that dir.
    create_dir(min_name);
    closedir(currDir);
}

int main () {
    int first_option, second_option, button = 1;
    char file_name[100];
    while (first_option != 2) {
        printf("\n1. Select file to process\n2. Exit the program\nEnter a choice 1 or 2: ");
        scanf("%d", &first_option);
        while (first_option != 1 && first_option != 2) {
                printf("\nInvalid option! Please enter again.\n\n1. Select file to process\n2. Exit the program\nEnter a choice 1 or 2: ");
                scanf("%d", &first_option);
            }
            if (first_option == 2) {
                // Option 2, exit program.
                exit(0);
            } else {
                // Option 1, goes to Selecting File.
                while (1) {
                    // deal with the third option when the inputted file doesn't show in current directory. 
                    printf("\nWhich file you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\nEnter a choice from 1 to 3: ");
                    scanf("%d", &second_option);
                    while (second_option != 1 && second_option != 2 && second_option != 3) {
                        printf("\nInvalid option! Please Enter again.\n\nWhich file you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\nEnter a choice from 1 to 3: ");
                        scanf("%d", &second_option);
                    }
                    if (second_option == 1) {
                        // User picks 1: Processing lagest file.
                        largest_finding();
                        break;
                    } else if (second_option == 2) {
                        // User picks 2: Processing smallest file.
                        smallest_finding();
                        break;
                    } else {
                        // User picks 3: Processing given file.
                        printf("\nEnter the complete file name: ");
                        scanf("%s", file_name);
                        DIR *currDir = opendir("./");
                        struct dirent *aDir;
                        struct stat *dirStat;
                        while ((aDir = readdir(currDir)) != NULL) {
                            if (strcmp(aDir->d_name, file_name) == 0) {
                                button = 1;
                                create_dir(file_name);
                                break;
                            } else 
                                button = 0;
                        }
                        if (button == 0) 
                            printf("\nFile cannot found!\n");
                        else
                            break;

                        closedir(currDir);
                    }
                }
                
            }
    }


}