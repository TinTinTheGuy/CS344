#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<fcntl.h>


struct Movies
{ /* struct for movies information */
    char *title;
    char *year;
    char *language;
    char *rating;
    struct Movies *next;
};


struct Movies *createMovies(char *currLine)
{
    struct Movies *currMovies =
    malloc(sizeof(struct Movies));

    char *saveptr;

    char *token = strtok_r(currLine, ",", &saveptr);
    // token for each of movie information
    currMovies->  title = calloc(strlen(token) + 1,
    sizeof(char));
    strcpy(currMovies->title, token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovies->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovies->year, token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovies->language = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovies->language, token);

    token = strtok_r(NULL, ",", &saveptr);
    currMovies->rating = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovies->rating, token);
    
    currMovies->next = NULL;

    return currMovies;


};
    // Follows module 1 example
struct Movies *processFile(char *filePath)
{
    // Open the specified file for reading only
    FILE *MoviesFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    // The head of the linked list
    struct Movies *head = NULL;
    // The tail of the linked list
    struct Movies *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, MoviesFile)) != -1)
    {
        // Get a new student node corresponding to the current line
        struct Movies *newNode = createMovies(currLine);

        // Is this the first node in the linked list?
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
    free(currLine);
    fclose(MoviesFile);
    return head;
}
// print data
void printMovies(struct Movies* aMovies){
   printf("%s, %s %s, %s\n",    aMovies->title,
                                aMovies->year,
                                aMovies->language,
                                aMovies->rating);
}

// print linkedlist
void printMoviesList(struct Movies *list)
{   
    while (list != NULL)
    {
        printMovies(list);
        list = list->next;
    }
}

int main(int argc, char *argv[])
{
        if (argc < 2)
            {
            printf("You must provide the name of the file to process\n");
            printf("Example usage: ./students student_info1.txt\n");
            return EXIT_FAILURE;
        }
        struct Movies *list = processFile(argv[1]);
        
        int choice = 0;
        while(choice != 4){
            printf("1. Show moviews released in the specified year \n");
            printf("2. Show highest rated movie for each year \n");
            printf("3. Show the title and year of release of all movies in a specific language \n");
            printf("4. Exit from the program  \n");
            printf(" Enter a choice from 1 to 4 :\n");

        scanf("%d", &choice);

    switch (choice) 
    {
        case 1:
                {
                int year;
                printf("Enter the year for which you want to see movies: ");
                scanf("%d", &year);

                struct Movies *curr = list;
                int found = 0;
                while (list != NULL)
                {
                    if (atoi(list->year) == year)
                    {
                        printf("%s\n", list->title);
                        found = 1;
                    }
                    list = list->next;
                }

                if (!found)
                {
                    printf("No data about movies released in the year %d\n", year);
                }

                break;
                }
        case 2:
            {       // Array as 122 because the year from 1900 to 2021 is 121
                    float Highrating[122] = {-1};
                    char* titleName[122] = {0};
                    struct Movies* newList = list;
                while (newList != NULL) { 
                    int year = atoi(newList->year) - 1900;
                    float rating = atof(newList->rating); 
                if (rating >= Highrating[year]) {
                    Highrating[year] = rating;
                    titleName[year] = newList->title;
                }
                    
                    // list = list -> next;   // When I do this, it can not repeat the case 2
                    newList = newList->next;
                                    }
                for (int i = 0; i < 122; i++) {
                    if (Highrating[i] > 0) {
                        printf("%d %.1f %s\n", (1900 + i), Highrating[i], titleName[i]);
                    }
                                              }
                break;
            }
            
        case 3:
                {
            char Language[20];
            printf("Enter the language for which you want to see movies:");
            scanf("%s", &Language);
            int Exit = 0;
            struct Movies* curr = list;
            while (curr != NULL) {
                if (curr->language != NULL && strlen(curr->language) > 0) { // check  valid string
                    char *token;
                    const char delim[] = "[];"; // delimiter characters
                    token = strtok(curr->language, delim);
                    
                    while (token != NULL) {
                        if (strcmp(Language, token) == 0){
                            printf("%s %s\n", curr->year, curr->title);
                            Exit = 1;
                            break; //Break if Exit is 1
                                                        }
                                
                            token = strtok(NULL, delim);
                                          }
                                                                            }
                    curr = curr->next;
                 }
            if(Exit == 0)
                printf("No data about movies released in %s\n", Language);
                }       
                

        case 4:
                {
                printf("Exiting the program...\n");
                break;
                }

            default:
                {
                printf("Invalid choice. Please enter a number from 1 to 4.\n");
                break;
                }        
}   
    }
}