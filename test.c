#include <stdio.h>

struct student
{
   int rollno;
   char *name;
   int m1, m2, m;
   float percent;
};
// Example: identifier as user defined type
struct student s1 = {1, "Raju", 50, 60, 70, 60.00};

// Example: identifier in typedef
typedef struct student STUDENT;
STUDENT s2 = {1, "Raju", 50, 60, 70, 60.00};

// Example: identifier in enum
enum week
{
   Mon = 10,
   Tue,
   Wed,
   Thur,
   Fri = 10,
   Sat = 16,
   Sun
};

union Data
{
   int i;
   float f;
   char star[20];
};

int main(int argc, char **argv)
{

   union Data data;
   data.i = 10;
   printf("%d----%d", data.i);

   /* my first program in C */
   // printf("Hello, World!%d%d%d", Mon, Wed , Fri);
   printf("%d----%d", argv, &argv);
   // int x=0;
   // begin:
   // x++;
   // if(x >=10)
   //    goto end;
   // printf("\n%d", x);
   // goto begin;
   // end:
   return 0;
}
int average(int marks1, int marks2)
{
   return (float)(marks1 + marks2) / 2;
}

void demo()
{
   static int count = 0;
   printf("%d", count);
   count++;
}
void demo2()
{
   int count = 0;
   printf("%d", count);
   count++;
}
int main2()
{
   int i = 0;
   for (; i < 5; i++)
   {
      demo();  // 0 0 0 0 0
      demo2(); // 0 1 2 3 4
   }
}

int main3()
{
   int a = 5;
   int b = 7;
   const int *pa = &a; // int const *pa = &a;
   // *pa = b;// error
   pa = &b; // ok

   printf("%d--%d\n", *pa, pa);
}

int main4()
{

   int a = 5;
   int b = 7;
   int *const pa = &a;
   *pa = b; // ok
   // pa = &b; // erro
   printf("%d--%d\n", *pa, pa);
}
int main4()
{

   int a = 5;
   int b = 7;
   int const *const pa = &a;
   // *pa = b;  // error
   // pa = &b; // error
   printf("%d--%d\n", *pa, pa);
}

#include <stdio.h>

int main(void)
{
   int a = 0;
   char greeting_a[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
   char greeting_b[] = "Hello";
   char *greeting_c = "Hello";
   for (int i = 0; i < 6; i++)
   {
      //  printf("%d", greeting_c);
      printf("%c", *(greeting_c + i));
   }
}

#include <stdio.h>

int main(void)
{
   int my_array[5];
   int my_array_b[4] = {0, 1, 2, 3, 4};

   size_t len = sizeof(my_array) / sizeof(my_array[0]);
   printf("%d-%d\n", sizeof(my_array), sizeof(my_array[0]));
   for (size_t i = 0; i < len; i++)
   {
      my_array[i] = i;
      printf("%d-%d-%d-%d-%d\n", i, my_array[i], my_array_b[i], my_array_b[5], my_array_b[6]);
   }
   // 20-4
   // 0-0-0-32764-100
   // 1-1-1-1-100
   // 2-2-2-1-2
   // 3-3-3-1-2
   // 4-4-0-1-2
}

void b(int *a)
{
   *a *= 2;
   printf("%d", *a);
}
int main(void)
{
   int a = 5;
   b(&a);
}

#include <stdio.h>

static inline void square(int *a)
{
   *a *= *a;
   printf("%d", *a);
   // return ;
}
int main(void)
{
   int num = 5;
   square(&num);
   // square(&num);
   printf("-%d\n", num);
}


inline int max(int a, int b)
{
   return a > b ? a : b;
}

int main()
{
   int x[5];
   int *x_pt = &x[0];
   int *x_p2 = x;
   printf("%d\n", (void *)x_pt);
   printf("%d", (void *)(x_pt + 1));
}



struct my_struct
{
   int x;
   int y;
};

union my_union
{
   int i;
   float f;
   char str[20];
};
typedef unsigned char byte1;
#define SIZE 5 
int main()
{
   struct my_struct object1;
   union my_union object2;
   object1.x = SIZE;
   object2.i = SIZE;
   printf("%d\n", object1.x);
   printf("%d\n", object2.i);

    byte1 b1 = 'c';
    printf("%c\n",b1);
}

int main() {
    unsigned char a = 5; // 00000101
    unsigned char b = 9; // 00001001
    printf("a & b = %d\n", a & b); // 00000001
    printf("a | b = %d\n", a | b); // 00001101
    printf("a ^ b = %d\n", a ^ b); // 00001100
    printf("~a = %d\n", a = ~a); // 11111010
    printf("b << 1 = %d\n", b << 1); // 00010010
    printf("b >> 1 = %d\n", b >> 1); // 00000100
    return 0;
}

#include <stdio.h>

enum week {
    mon,
    tue,
    wed,
    thu,
    fri,
    sat,
    sun
};

int main() {
    for(int i = mon; i <= sun; i++) {
        printf("%d\n", i); 
    } 
    return 0;
}

#include <stdio.h>
#include <stdarg.h>
int a( int a, ...) {
    
    va_list args;
      printf("%d-%d-%d-%d\n",args,args[2],args[1], args[2]);
      
      va_start(args, 5);
        for(int i=0; i < 5;i++) {
            int get_args = va_arg(args,int);
            printf("%d\n", get_args);
        }
      va_end(args);
    return 1;
}

int main()
{
    
    int b= a(1,2,3,4,5);
    printf("\n%d", b);
    return 0;
}

// 1496470704-3-4-5 ?
// 2
// 3
// 4
// 5
// -1046781888

// 1


#include <stdio.h>

// Hàm này cố gắng sửa đổi địa chỉ của con trỏ được truyền vào
void modifyString( const char *sPtr) {
    // sPtr[0] = 'a'; // Lỗi: Cố gắng sửa đổi con trỏ hằng
    for (; *sPtr != '\0'; ++sPtr) { // no initialization
      printf("%c", *sPtr);                             
   }    
   
}

int main() {
    char message[] = "Hello, world!";

    // Gọi hàm modifyString với con trỏ không hằng char *sPtr
    modifyString(message); // Lỗi: Cố gắng sửa đổi con trỏ hằng
   for (int i = 0; i < 6; i++)
   {
      //  printf("%d", greeting_c);
      printf("%c", *(message + i));
   }
    return 0;
}
#include <stdio.h>


int main(void) {
   int x = 5; 
   int y = 0; 

   // ptr is a constant pointer to a constant integer. ptr always 
   // points to the same location; the integer at that location
   // cannot be modified
   const int *const ptr = &x; // initialization is OK
                                 
   printf("%d\n", *ptr);
   // *ptr = 7; // error: *ptr is const; cannot assign new value 
   // ptr = &y; // error: ptr is const; cannot assign new address
} 

#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *left;
    struct Node *right;
} Node;

Node* create_node(int data) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* insert_node(Node* node, int data) {
    //   printf("%dz%dz%dz\n ", node->data);
    if (node != NULL)  printf("node->data:%d\n", node->data);
  
    if (node == NULL) {
         printf("vvvvvvvvvvvvvvvvvvvvvvv\n");
        return create_node(data);
    } else if (data < node->data) {
        printf("%d-%d-%d-left\n",data, node->data, node->left);
        node->left = insert_node(node->left, data);
    } else if (data > node->data) {
        printf("%d-%d-%d-right\n",data, node->data, node->right);
        node->right = insert_node(node->right, data);
    }
     printf("-------------\n");
    return node;
}

void inorder_traversal(Node* node) {
    if (node != NULL) {
        inorder_traversal(node->left);
        printf("%d ", node->data);
        printf("%d ", node->left);
        printf("%d\n ", node->right);
        inorder_traversal(node->right);
    }
}

int main() {
    Node* root = NULL;
    root = insert_node(root, 50);
    insert_node(root, 30);
    insert_node(root, 20);
    insert_node(root, 40);
    insert_node(root, 60);
    insert_node(root, 10);

    inorder_traversal(root);

    return 0;
}

#include <stdio.h>
// A function with an const char pointer parameter
// and void return type
void DisplayMessage(const char *msg)
{
    printf("Message  =>>  %s\n", msg);
}
int main()
{
    // pfDisplayMessage is a pointer to function DisplayMessage()
    void ( *pfDisplayMessage) (const char *) = &DisplayMessage;
    // Invoking DisplayMessage() using pfDisplayMessage
    (*pfDisplayMessage)("Hello Aticleworld.com");
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
//function used to add two numbers
int AddTwoNumber(int iData1,int iData2)
{
    return (iData1 + iData2);
}
int main(int argc, char *argv[])
{
    int iRetValue = 0;
    //Declaration of function pointer
    int (*pfAddTwoNumber)(int,int) = NULL;
    //initialize the function pointer
    pfAddTwoNumber = AddTwoNumber;
    //Calling the function using the function pointer
    iRetValue = (*pfAddTwoNumber)(10,20);
    //display addition of two number
    printf("\n\nAddition of two number = %d\n\n",iRetValue);
}

#include <stdio.h>
typedef  int (*pfunctPtr)(int, int); /* function pointer */
//function pointer as arguments
int ArithMaticOperation(int iData1,int iData2, pfunctPtr Calculation)
{
    int iRet =0;
    iRet = Calculation(iData1,iData2);
    return iRet;
}
/*function add two number*/
int AddTwoNumber(int iData1,int iData2)
{
    return (iData1 + iData2);
}
/*function subtract two number*/
int SubTwoNumber(int iData1,int iData2)
{
    return (iData1 - iData2);
}
/*function multiply two number*/
int MulTwoNumber(int iData1,int iData2)
{
    return (iData1 * iData2);
}
int main()
{
    int iData1 = 0;
    int iData2 = 0;
    int iChoice = 0;
    int Result = 0;
    printf("Enter two Integer Data \n\n");
    scanf("%d%d",&iData1,&iData2);
    printf("Enter 1 for Addition \n\n");
    printf("Enter 2 for Subtraction \n\n");
    printf("Enter 3 for Multiplication \n\n");
    printf("User choice :");
    scanf("%d",&iChoice);
    switch(iChoice)
    {
    case 1:
        Result = ArithMaticOperation(iData1,iData2,AddTwoNumber);
        break;
    case 2:
        Result = ArithMaticOperation(iData1,iData2,SubTwoNumber);
        break;
    case 3:
        Result = ArithMaticOperation(iData1,iData2,MulTwoNumber);
        break;
    default:
        printf("Enter Wrong Choice\n\n");
    }
    printf("\n\nResult  = %d\n\n",Result);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
//Add two number
int AddTwoNumber(int iData1,int iData2)
{
    return (iData1 + iData2);
}
//Subtract two number
int SubTwoNumber(int iData1,int iData2)
{
    return (iData1 - iData2);
}
//Multilply two number
int MulTwoNumber(int iData1,int iData2)
{
    return (iData1 * iData2);
}
// Main function
int main(int argc, char *argv[])
{
    int iRetValue = 0;
    //Declaration of array of function pointer
    int (*apfArithmatics [3])(int,int) = {AddTwoNumber,SubTwoNumber,MulTwoNumber};
    //Calling the Add function using index of array
    iRetValue = (*apfArithmatics [0])(20,10);
    //display addition of two number
    printf("\n\nAddition of two number = %d\n\n",iRetValue);
    //Calling the subtract function using index of array
    iRetValue = (*apfArithmatics[1])(20,10);
    //display subtraction of two number
    printf("\n\nsubtraction of two number = %d\n\n",iRetValue);
    //Calling the multiply function using index of array
    iRetValue = (*apfArithmatics[2])(20,10);
    //display multiplication  of two number
    printf("\n\nmultiplication of two number = %d\n\n",iRetValue);
    return 0;
}

#include <stdio.h>
#define ELEMENT_SIZE(x)  sizeof(x[0])
#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))
//compare function for intger array

int compareInt(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y)
        return -1;  //-1 for ascending, 1 for descending order.
    else if (x > y)
        return 1;   //1 for ascending, -1 for descending order.
    return 0;
}
//compare function for float array
int compareFloat(const void *a, const void *b)
{
    float x = *(const float *)a;
    float y = *(const float *)b;
    if (x < y)
        return -1;  //-1 for ascending, 1 for descending order.
    else if (x > y)
        return 1;   //1 for ascending, -1 for descending order.
    return 0;
}
int main(int argc, char *argv[])
{
    //Integer array
    int iData[] = { 40, 10, 100, 90, 20, 25 };
    //float array
    float fData[] = {1.2,5.7,78,98.5,45.67,81.76};
    //array index
    int index = 0;
    //sorting integer array
    qsort(iData,ARRAY_SIZE(iData),ELEMENT_SIZE(iData),compareInt);
    for (index=0; index<ARRAY_SIZE(iData); index++)
    {
        printf ("%d ",iData[index]);
    }
    printf("\n\n");
    //sortig float array
    qsort(fData,ARRAY_SIZE(fData),ELEMENT_SIZE(fData),compareFloat);
    for (index=0; index<ARRAY_SIZE(fData); index++)
    {
        printf ("%f ",fData[index]);
    }
    return 0;
}

void qsort(void *arr, size_t elements, size_t size, int (*comp)(const void *, const void*));


#include <stdio.h>

// card structure definition            
struct card {                           
   const char *face; // define pointer face   
   const char *suit; // define pointer suit
};                                 

int main(void) {
   struct card myCard; // define one struct card variable   

   // place strings into myCard
   myCard.face = "Ace";   
   myCard.suit = "Spades";

   struct card *cardPtr = &myCard; // assign myCard's address to cardPtr

   printf("%s of %s\n", myCard.face, myCard.suit);
   printf("%s of %s\n", cardPtr->face, cardPtr->suit);
   printf("%s of %s\n", (*cardPtr).face, (*cardPtr).suit);
}


// fig10_02.c
// Card shuffling and dealing program using structures
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CARDS 52
#define FACES 13

// card structure definition                  
struct card {                                 
   const char *face; // define pointer face   
   const char *suit; // define pointer suit   
};


typedef struct card Card; // new type name for struct card   

// prototypes
void fillDeck(Card * const deck, const char *faces[], const char *suits[]);
void shuffle(Card * const deck);
void deal(const Card * const deck);

int main(void) {
   Card deck[CARDS]; // define array of Cards

   // initialize faces array of pointers
   const char *faces[] = { "Ace", "Deuce", "Three", "Four", "Five",
      "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

   // initialize suits array of pointers
   const char *suits[] = { "Hearts", "Diamonds", "Clubs", "Spades"};

   srand(time(NULL)); // randomize

   fillDeck(deck, faces, suits); // load the deck with Cards
    printf("1111%d\n", deck);
    for (size_t i = 0; i < CARDS; ++i) {
      printf("%5s of %-8s%s", deck[i].face, deck[i].suit,
         (i + 1) % 4 ? "  " : "\n");
   }
   shuffle(deck); // put Cards in random order
    printf("22222%d\n", deck);
    for (size_t i = 0; i < CARDS; ++i) {
      printf("%5s of %-8s%s", deck[i].face, deck[i].suit,
         (i + 1) % 4 ? "  " : "\n");
   }
   printf("3333333%d\n", deck);
   deal(deck); // deal all 52 Cards
 
}

// place strings into Card structures
void fillDeck(Card * const deck, const char * faces[], 
   const char * suits[]) {
   // loop through deck
   for (size_t i = 0; i < CARDS; ++i) { 
      deck[i].face = faces[i % FACES];
      deck[i].suit = suits[i / FACES];
   }
}

// shuffle cards
void shuffle(Card * const deck) {
   // loop through deck randomly swapping Cards
   for (size_t i = 0; i < CARDS; ++i) { 
      size_t j = rand() % CARDS;
      Card temp = deck[i];
      deck[i] = deck[j];  
      deck[j] = temp;     
   }
}

// deal cards
void deal(const Card * const deck) {
   // loop through deck
   for (size_t i = 0; i < CARDS; ++i) {
      printf("%5s of %-8s%s", deck[i].face, deck[i].suit,
         (i + 1) % 4 ? "  " : "\n");
   }
}


#include <stdio.h>
#define CARDS 52

// bitCard structure definition with bit fields
struct bitCard {                          
   unsigned int face : 4; // 4 bits; 0-15 
   unsigned int suit : 2; // 2 bits; 0-3  
   unsigned int color : 1; // 1 bit; 0-1  
};                                        

typedef struct bitCard Card; // new type name for struct bitCard   

void fillDeck2(Card * const deck); // prototype
void deal(const Card * const deck); // prototype

int main(void) {
   Card deck[CARDS]; // create array of Cards

   fillDeck2(deck);

   puts("Card values 0-12 correspond to Ace through King");
   puts("Suit values 0-3 correspond to Hearts, Diamonds, Clubs and Spades");
   puts("Color values 0-1 correspond to red and black\n");
   deal(deck);
}

// initialize Cards
void fillDeck2(Card * const deck) {
   // loop through deck
   for (size_t i = 0; i < CARDS; ++i) { 
      deck[i].face = i % (CARDS / 4); 
      deck[i].suit = i / (CARDS / 4); 
      deck[i].color = i / (CARDS / 2);
   }
}

// output cards in two-column format; cards 0-25 indexed with 
// k1 (column 1); cards 26-51 indexed with k2 (column 2)
void deal(const Card * const deck) {
   // loop through deck
   for (size_t k1 = 0, k2 = k1 + 26; k1 < CARDS / 2; ++k1, ++k2) { 
      printf("Card:%3d  Suit:%2d  Color:%2d   ",
         deck[k1].face, deck[k1].suit, deck[k1].color);
      printf("Card:%3d  Suit:%2d  Color:%2d\n",
         deck[k2].face, deck[k2].suit, deck[k2].color);
   }
}

#include <stdio.h>

// enumeration constants represent months of the year            
enum months {                                                     
   JAN = 1, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC 
};                                                                
 
int main(void) {
   // initialize array of pointers
   const char *monthName[] = { "", "January", "February", "March", 
      "April", "May", "June", "July", "August", "September", "October",
      "November", "December" };
   
   // loop through months
   for (enum months month = JAN; month <= DEC; ++month) {
      printf("%2d%11s\n", month, monthName[month]);
   }
}

#include <stdio.h>

// Định nghĩa kiểu dữ liệu cJSON_bool
typedef int cJSON_bool;

// Định nghĩa hằng số true và false
//#define true  ((cJSON_bool)1)
#define true  ((cJSON_bool)2)
#define false ((cJSON_bool)0)


int main() {
    cJSON_bool flag = 2;
    if (flag == true) {
        printf("The flag is true.\n");
    } else {
        printf("The flag is false.\n");
    }
    
     cJSON_bool flag2 = true;
   if (flag2 == true) {
        printf("The flag is true.\n");
    } else {
        printf("The flag is false.\n");
    }
    
    return 0;
}

#include <stdio.h>

// Định nghĩa cấu trúc listNode
struct listNode {
    char data;              // Dữ liệu của nút
    struct listNode *next;  // Con trỏ đến nút tiếp theo
};

// Định nghĩa đồng nghĩa ListNode cho cấu trúc listNode
typedef struct listNode ListNode;

// Định nghĩa đồng nghĩa ListNodePtr cho con trỏ đến listNode
typedef ListNode *ListNodePtr;

int main() {
    // Khai báo một danh sách liên kết đơn rỗng
    ListNodePtr head = NULL;

    // Thêm một nút mới vào danh sách
    ListNodePtr newNode = malloc(sizeof(ListNode));
    newNode->data = 'A';
    newNode->next = NULL;
    head = newNode;

    // In dữ liệu của nút đầu tiên trong danh sách
    if (head != NULL) {
        printf("Data of the first node: %c\n", head->data);
    }

    return 0;
}
// fig15_05.c 
// Using the goto statement
#include <stdio.h>

int main(void) {
   int count = 1; // initialize count

   start: // label
      if (count > 10) {
         goto end;
      } 

      printf("%d  ", count);
      ++count;

      goto start; // goto start on line 9

   end: // label
      putchar('\n');
} 

#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    int size = sizeof(arr) / sizeof(arr[0]);
    
    // Duyệt qua từng phần tử của mảng
    for (int i = 0; i < size; i++) {
        // Chuyển đổi từng phần tử thành chuỗi ký tự
        char str[20]; // Độ dài tối đa của chuỗi
        printf("%d", arr[i]);
        sprintf(str, "--%d", arr[i]);
        
        // In chuỗi ký tự bằng hàm puts
        puts(str);
    }
    char str[] = "Hello, world!";
    puts(str); 
    return 0;
}

#include <stdio.h>
int main(void) {
   printf("__LINE__ = %d\n", __LINE__);
   printf("__FILE__ = %s\n", __FILE__);
   printf("__DATE__ = %s\n", __DATE__);
   printf("__TIME__ = %s\n", __TIME__);
   printf("__STDC__ = %d\n", __STDC__);
} 


// fig12_01.c
// Inserting and deleting nodes in a list
#include <stdio.h>
#include <stdlib.h>

// self-referential structure                       
struct listNode {                                    
   char data; // each listNode contains a character  
   struct listNode *nextPtr; // pointer to next node 
};                                                   

typedef struct listNode ListNode; // synonym for struct listNode
typedef ListNode *ListNodePtr; // synonym for ListNode*

// prototypes
void insert(ListNodePtr *sPtr, char value);
char delete(ListNodePtr *sPtr, char value);
int isEmpty(ListNodePtr sPtr);
void printList(ListNodePtr currentPtr);
void instructions(void);

int main(void) {
   ListNodePtr startPtr = NULL; // initially there are no nodes
   char item = '\0'; // char entered by user

   instructions(); // display the menu
   printf("%s", "? ");
   int choice = 0; // user's choice
   scanf("%d", &choice);

   // loop while user does not choose 3
   while (choice != 3) { 
      switch (choice) { 
         case 1: // insert an element
            printf("%s", "Enter a character: ");
            scanf("\n%c", &item);
            insert(&startPtr, item); // insert item in list
            printList(startPtr);
            break;
         case 2: // delete an element
            if (!isEmpty(startPtr)) { // if list is not empty
               printf("%s", "Enter character to be deleted: ");
               scanf("\n%c", &item);

               // if character is found, remove it
               if (delete(&startPtr, item)) { // remove item
                  printf("%c deleted.\n", item);
                  printList(startPtr);
               } 
               else {
                  printf("%c not found.\n\n", item);
               } 
            } 
            else {
               puts("List is empty.\n");
            } 

            break;
         default:
            puts("Invalid choice.\n");
            instructions();
            break;
      }

      printf("%s", "? ");
      scanf("%d", &choice);
   } 

   puts("End of run.");
} 

// display program instructions to user
void instructions(void) {
   puts("Enter your choice:\n"
      "   1 to insert an element into the list.\n"
      "   2 to delete an element from the list.\n"
      "   3 to end.");
} 

// insert a new value into the list in sorted order
void insert(ListNodePtr *sPtr, char value) {
   ListNodePtr newPtr = malloc(sizeof(ListNode)); // create node

   if (newPtr != NULL) { // is space available?
      newPtr->data = value; // place value in node
      newPtr->nextPtr = NULL; // node does not link to another node

      ListNodePtr previousPtr = NULL;
      ListNodePtr currentPtr = *sPtr;
      // loop to find the correct location in the list        
      while (currentPtr != NULL && value > currentPtr->data) {
         previousPtr = currentPtr; // walk to ...             
         currentPtr = currentPtr->nextPtr; // ... next node   
      }                                                       

      // insert new node at beginning of list
      if (previousPtr == NULL) { 
         newPtr->nextPtr = *sPtr;
         *sPtr = newPtr;
      } 
      else { // insert new node between previousPtr and currentPtr
         previousPtr->nextPtr = newPtr;
         newPtr->nextPtr = currentPtr;
      } 
   } 
   else {
      printf("%c not inserted. No memory available.\n", value);
   } 
} 

// delete a list element
char delete(ListNodePtr *sPtr, char value) {
   // delete first node if a match is found
   if (value == (*sPtr)->data) { 
      ListNodePtr tempPtr = *sPtr; // hold onto node being removed
      *sPtr = (*sPtr)->nextPtr; // de-thread the node
      free(tempPtr); // free the de-threaded node
      return value;
   } 
   else { 
      ListNodePtr previousPtr = *sPtr;
      ListNodePtr currentPtr = (*sPtr)->nextPtr;

      // loop to find the correct location in the list
      while (currentPtr != NULL && currentPtr->data != value) { 
         previousPtr = currentPtr; // walk to ...  
         currentPtr = currentPtr->nextPtr; // ... next node  
      } 

      // delete node at currentPtr
      if (currentPtr != NULL) { 
         ListNodePtr tempPtr = currentPtr;
         previousPtr->nextPtr = currentPtr->nextPtr;
         free(tempPtr);
         return value;
      } 
   } 

   return '\0';
} 

// return 1 if the list is empty, 0 otherwise
int isEmpty(ListNodePtr sPtr) {
   return sPtr == NULL;
} 

// print the list
void printList(ListNodePtr currentPtr) {
   // if list is empty
   if (isEmpty(currentPtr)) {
      puts("List is empty.\n");
   } 
   else { 
      puts("The list is:");

      // while not the end of the list
      while (currentPtr != NULL) { 
         printf("%c --> ", currentPtr->data);
         currentPtr = currentPtr->nextPtr;   
      } 

      puts("NULL\n");
   } 
} 




#include <stdio.h>
#include <stdlib.h>

int main() {
    // Ví dụ sử dụng malloc()
    int *ptr1 = (int *)malloc(5 * sizeof(int));
    if (ptr1 == NULL) {
        printf("Không thể cấp phát bộ nhớ.\n");
        return 1;
    }
    
    // Ví dụ sử dụng calloc()
    int *ptr2 = (int *)calloc(5, sizeof(int));
    if (ptr2 == NULL) {
        printf("Không thể cấp phát bộ nhớ.\n");
        return 1;
    }
    
    // Ví dụ sử dụng realloc() để thay đổi kích thước của vùng nhớ
    int *ptr3 = (int *)realloc(ptr1, 10 * sizeof(int));
    if (ptr3 == NULL) {
        printf("Không thể thay đổi kích thước bộ nhớ.\n");
        free(ptr1); // Giải phóng bộ nhớ đã cấp phát trước đó
        return 1;
    }
    
    // Sử dụng bộ nhớ được cấp phát
    for (int i = 0; i < 10; i++) {
        ptr3[i] = i;
    }
    
    // In ra các giá trị
    printf("Cac gia tri trong mang sau khi su dung realloc():\n");
    for (int i = 0; i < 10; i++) {
        printf("%d ", ptr3[i]);
    }
    printf("\n");
    
    // Giải phóng bộ nhớ đã cấp phát
    free(ptr2);
    free(ptr3);
    
    return 0;
}

#include<stdio.h>
void fun(int *p)
{
  int q = 40;
  p = &q;
}
int main()
{
  int data = 27;
  int *ptr = &data;
  fun(ptr);
  printf("%d", *ptr);
  return 0;
}
// => 27

#include<stdio.h>
void fun(int **p)
{
  static int q = 40;
  *p = &q;
}
int main()
{
  int data = 27;
  int *ptr = &data;
  fun(&ptr);
  printf("%d", *ptr);
  return 0;
}
// => 40