#include "gng1106plplot.h"
// Define symbolic constants
#define TRUE 1
#define FALSE 0
#define MAX_SIZE 100
#define NUM_SAVES 5
#define SAVE_FILE "savefile.txt"
#define SMALL_NUMBER 0.000001
// Structure definintions
typedef struct
{
    //input from user
    double v1, v2, v3; //reactor volume
    double Q01, Q03, Q12, Q23, Q31, Q33; //flow through channels
    double put1, put2; //input concentrations
    double in1, in2, in3; //initial concentration at time(s)
    double deltat; //time step for euler method
    double tfinal; //end time for plotting results
    //calculated values
    double n; //number of values to compute
    //must be less than MAX_SIZE
    double t[MAX_SIZE]; //array containing the times for the plot
    double c1[MAX_SIZE]; //array containing concentrations over time for reactor 1
    double c2[MAX_SIZE]; //array containing concentrations over time for reactor 2
    double c3[MAX_SIZE]; //array containing concentrations over time for reactor 3
    int filled; // boolean that checks if a save slot is filled or not
} REACTOR;
// function prototypes
int getFileValue(REACTOR saves[NUM_SAVES], REACTOR *);
void getUserInput(REACTOR *);
void saveFile(REACTOR saves[NUM_SAVES], REACTOR *);
double getPositiveValue(char *);
void calculateConcentrationEuler(REACTOR *);
void plot(REACTOR *);
double getMin(double *, double);
double getMax(double *, double);
/*--------------------------------------------
Function: main
Description:  Overall control of the program.
Gets the input from the user, calculates concentration for each
reactor using Euler's method and plots the 3 curves.
----------------------------------------------*/
void main ()
{
    REACTOR reactor; // input and output data
    REACTOR saves[NUM_SAVES]; //stores possible reactor value
    int load = FALSE; //stores boolean integer of whether save was loaded or not
    load = getFileValue(saves, &reactor); //ask user if they want to use previous save
    //if the user did not choose a save file, then program asks user for input
    if(load == FALSE)
    {
        //get the user input
        getUserInput(&reactor);
        saveFile(saves, &reactor);
    }
    //Calculations
    calculateConcentrationEuler(&reactor);
    //plotting
    plot(&reactor);
}
/*---------------------------------------------------------
Function: getFileValue
Description: Checks for saves and prompts user if they want to
use a save
Parameters:
ptr - reference to REACTOR structure variable.
saves[5] - array of 5 reactors
Return Value: integer value stating whether save file was used
-------------------------------------------------------------*/
int getFileValue(REACTOR saves[NUM_SAVES], REACTOR *ptr)
{
    FILE *fp; // Pointer to a FILE structure
    int i; //counter for saves
    int boole; //stores value of whether user loaded a save or not
    fp = fopen(SAVE_FILE,"rb"); // Open for read
    //checks if save file exists
    if(fp == NULL)
    {
        boole = FALSE; //user did not load a save file
        printf("Save file does not exist or is empty. Must input new data\n");
    }
    else
    {
        fread(saves, sizeof(REACTOR),5 ,fp);
        //welcome message
        printf("FILES:\n");
        //for loop that runs 5 times
        for(i = 0; i < NUM_SAVES; i++)
        {
            //reads the values from the file into the reactor array
            //fread(&saves[i], sizeof(REACTOR),1 ,fp);
            //checks which save files are empty or full
            if(saves[i].filled == TRUE)
                printf("Slot %d contains a save\n", i+1);
            else
                printf("Slot %d is empty\n", i+1);
        }

        //asks user to select a save file
        do
        {
            printf("\nIf you would like to access a saved file, please enter an integer from 1-5.\n");
            printf("To calculate a new transient response of coupled chemical reactors, enter 0.\n");
            scanf("%d", &i);//reads user input and saves to integer ix
            //doesn't save file if user selected 0
            if(i !=0)
            {
                boole = TRUE; //user used data from a save file
                *ptr = saves[i-1]; //sets reactor to proper save file
            }
        }while(saves[i-1].filled != TRUE && i != 0);
    }
    fclose(fp); //closes file after use
    return boole; //tells porgram whether it should run the getUserInput and saveFile functions
}
/*----------------------------------------------------------
Function: getUserInput
Parameters:
rPtr - reference to REACTOR structure variable.
v - reactor volume
Q - flow rate through channels
iptC - input concentration
itlC - initial concentration at time(s)
tf - final time
Description: Gets user input for the structure variables in
REACTOR and calls on function getPositiveValue to ensure all
values are greater than zero.
-------------------------------------------------------------*/
void getUserInput(REACTOR *rPtr)
{
    int check;
    check = FALSE;
    //get reactor volume values
    printf("Reactor Volumes:\n");
    rPtr->v1 = getPositiveValue("Reactor 1");
    rPtr->v2 = getPositiveValue("Reactor 2");
    rPtr->v3 = getPositiveValue("Reactor 3");
    //get reactor flow rate values
    printf("Flow Rates:\n");
    //check that flow rates in REACTOR 1 are within volume constraints
    do
    {
        rPtr->Q01 = getPositiveValue("Channel Q-01");
        rPtr->Q31 = getPositiveValue("Channel Q-31");
        rPtr->Q12 = getPositiveValue("Channel Q-12");

        if (rPtr->Q01 + rPtr->Q31 - rPtr->Q12 < SMALL_NUMBER && rPtr->Q01 + rPtr->Q31 - rPtr->Q12 > -SMALL_NUMBER)
        {
            check = TRUE;
        }
        else
        {
            check = FALSE;
            printf("Please input flow rates that follow volume constraints, refer to equation 4 of project guidelines and try again.\n");
        }
    }while (check == FALSE);
    //check that flow rates in REACTOR 2 are within volume constraints
    do
    {
        rPtr->Q23 = getPositiveValue ("Channel Q-23");
        if (rPtr->Q23 == rPtr->Q12)
        check = TRUE;
        else
        {
            check = FALSE;
            printf("Channel Q-23's flow rate must be equal to Channel Q-12's flow rate to follow constraints. Please try again.\n");
        }
    }while (check == FALSE);
    //check if flow rates in reactor 3 are within volume constraints
    do
    {
        rPtr->Q33 = getPositiveValue ("Channel Q-33");
        if (rPtr->Q01 + rPtr->Q23 - rPtr->Q31 - rPtr->Q33 < SMALL_NUMBER && rPtr->Q01 + rPtr->Q23 - rPtr->Q31 - rPtr->Q33 > -SMALL_NUMBER)
            check = TRUE;
        else
        {
            check = FALSE;
            printf("Please input flow rates that follow volume constraints, refer to equation 4 of project guidelines and try again.\n");
        }
    }while (check == FALSE);
    //second check if flow rates in reactor 3 are within volume constraints
    do
    {
        rPtr->Q03 = getPositiveValue ("Channel Q-03");
        if (rPtr->Q01 + rPtr->Q03 - rPtr->Q33 < SMALL_NUMBER && rPtr->Q01 + rPtr->Q03 - rPtr->Q33 > -SMALL_NUMBER)
            check = TRUE;
        else
        {
            check = FALSE;
            printf("Please input flow rates that follow volume constraints, refer to equation 4 of project guidelines and try again.\n");
        }
    }while (check == FALSE);
    printf("Initial concentrations:\n");
    rPtr->in1 = getPositiveValue ("Concentration c-1,0");
    rPtr->in2 = getPositiveValue ("Concentration c-2,0");
    rPtr->in3 = getPositiveValue ("Concentration c-3,0");
    printf("Input concentrations:\n");
    rPtr->put1 = getPositiveValue ("Concentration c-01");
    rPtr->put2 = getPositiveValue ("Concentration c-03");
    printf("End Time:\n");
    do
    {
        rPtr->tfinal = getPositiveValue ("Final time");
        rPtr->deltat = getPositiveValue ("Time step");
        rPtr->n = rPtr->tfinal/rPtr->deltat;
        //printf("%lf\n", rPtr->n);
        if (rPtr->n > MAX_SIZE)
            printf("time step too small for final time (%lf)\n", rPtr->n);
    }while(rPtr->n > MAX_SIZE);

    rPtr->filled = TRUE;
}

/*----------------------------------------------------------
Function: saveFile
Description: Checks for saves and prompts user if they want to
save their file then saves files
Parameters:
ptr - reference to REACTOR structure variable.
saves[5] - array of 5 reactors
Return Value: N/A
-------------------------------------------------------------*/
void saveFile(REACTOR saves[NUM_SAVES], REACTOR *rPtr)
{
    int ix; //counter for saves
    //checks which save files are empty or full
    for(ix = 0; ix < NUM_SAVES; ix++)
    {
        if(saves[ix].filled == TRUE)
            printf("Slot %d contains a save\n", ix+1);
        else
            printf("Slot %d is empty\n", ix+1);
    }
    //asks user if they would like to save a save file
    do
    {
        printf("If you would like to save, select a save file from 1-5. \nTo not save press 0\n");
        scanf("%d", &ix); //reads user input and saves to integer ix
        //doesn't save file if user selected 0
        if(ix != 0);
            saves[ix-1] = *rPtr; //sets reactor to proper save file
    }while(saves[ix-1].filled != TRUE && ix != 0);
    if(ix != 0)
    {
        FILE *filePtr; // Pointer to a FILE structure
        filePtr = fopen(SAVE_FILE,"wb"); // Open for write
        //checks if save file exists
        if(filePtr == NULL)
            printf("Save file does not exist or is empty\n");
        else
        {
            fwrite(saves, sizeof(REACTOR),5 ,filePtr);
        }
        fclose(filePtr);
    }
}
/*----------------------------------------------------------
Function: getPositiveValue
Description: Prompt the user for a value (using the prompt string)
and checks that the value is positive
Return Value: Values that are greater than zero.
-------------------------------------------------------------*/
double getPositiveValue(char *prompt)
{
    double value; //value entered by the user
    do
    {
        printf("    Please enter a value for %s: ", prompt);
        scanf("%lf",&value);
        if (value <= 0.0)
            printf("The value must be greater than zero.\n");
    }while (value <= 0.0);
    return (value);
}
/*----------------------------------------------------------
Function: calculateConcentraion
Description: Fill the arrays with the data points for a concentration
time graph for all 3 reactors
Return Value: N/A
-------------------------------------------------------------*/
void calculateConcentrationEuler(REACTOR *rPtr)
{
    double time;    //time value
    int i;      //increment number
    double f_t;     //for computing f(t)
    //initializing values ar time 0
    time = 0.0;
    rPtr->t[0] = time;  //set time pointer equal to time
    rPtr->c1[0] = rPtr->in1;    //set reactor 1 concentration equal to initial concentration
    rPtr->c2[0] = rPtr->in2;    //set reactor 2 concentration equal to initial concentration
    rPtr->c3[0] = rPtr->in3;    //set reactor 3 concentration equal to initial concentration
    for(i = 1; i < rPtr->n; i = i + 1)
    {
        time = time + rPtr->deltat;
        rPtr->t[i] = time;
        //concentration of reactor 1
        f_t = (rPtr->Q01 * rPtr->put1 + rPtr->Q31 * rPtr->c3[i-1] - rPtr->Q12 * rPtr->c1[i-1]);
        f_t =  f_t / rPtr->v1;
        rPtr->c1[i] = rPtr->c1[i-1] + f_t*rPtr->deltat;
        //concentration of reactor 2
        f_t =  rPtr->c2[i] = (rPtr->Q12 * rPtr->c1[i-1] - rPtr->Q23 * rPtr->c2[i-1]);
        f_t =  f_t / rPtr->v2;
        rPtr->c2[i] = rPtr->c2[i-1] + f_t*rPtr->deltat;
        //concentration of reactor 3
        f_t = rPtr->c3[i] = (rPtr->Q23 * rPtr->c2[i-1] + rPtr->Q03 * rPtr->put2 - rPtr->Q31 * rPtr->c3[i-1] - rPtr->Q33 * rPtr->c3[i-1]);
        f_t =  f_t / rPtr->v3;
        rPtr->c3[i] = rPtr->c3[i-1] + f_t*rPtr->deltat;
    }
}

/*----------------------------------------------------------------------
Function: plot
Parameters:
rPtr - reference to REACTOR structure variable.
v - reactor volume
Q - flow rate through channels
iptC - input concentration
itlC - initial concentration at time(s)
tf - final time
Description: Initilialises the plot device,pen width,
             and plots both curves for the three concentrations.
----------------------------------------------------------------------*/
void plot(REACTOR *rPtr)
{
    double maxC, temp; // max concentration
    char plotLabel [100];
    //find maximum concentration to scale the concentration axis
    maxC = getMax(rPtr->c1, rPtr->n);   //get max concentration from reactor 1
    temp = maxC;    //set max concentration equal to temp variable
    maxC = getMax(rPtr->c2, rPtr->n); //get max concentration from reactor 2
    if(maxC > temp) temp = maxC;    //check if reactor 2 has the greatest concentration
        maxC = getMax(rPtr->c3, rPtr->n); //get max concentration from reactor 3
    if(maxC > temp) temp = maxC;    //check if reactor 3 has maximum concentration
        maxC = temp;
        maxC = maxC * 1.1;
    // initialize PLplot page
    plsdev("wingcc");   //set device to wingcc - codeblocks compiler
    //initialize the plot
    plinit();
    plwidth(2); //pen width
    plenv(0, rPtr->tfinal, 0, maxC, 0, 0);
    plcol0(BLACK);  //select colour for labels
    sprintf(plotLabel, "Concentration");
    pllab("Time t", "Concentration c(t)", plotLabel);
    //  plot curve for reactor 1
    plcol0(BLUE);
    pllsty(SOLID);
    plline(rPtr->n, rPtr->t, rPtr->c1);
    plptex(0.1*rPtr->tfinal, 0.9*maxC, 0, 0, 0, "Reactor 1");
    // plot curve for reactor 2
    plcol0(RED);
    pllsty(SOLID);
    plline(rPtr->n, rPtr->t, rPtr->c2);
    plptex(0.3*rPtr->tfinal, 0.9*maxC, 0, 0, 0, "Reactor 2");
    // plot curve for reactor 3
    plcol0(GREEN);
    pllsty(SOLID);
    plline(rPtr->n, rPtr->t, rPtr->c3);
    plptex(0.5*rPtr->tfinal, 0.9*maxC, 0, 0, 0, "Reactor 3");
    plend();
}

/*----------------------------------------------------------
Function: getMin
Parameters:
    array - reference to an array with double values
    n - number of elements in the array
Returns
    min:  the minimum value found in the array
Description: Traverses the array to find its minimum value.
----------------------------------------------------------------*/
double getMin(double *array, double n)
{
    int ix;
    double min = array[0];
    for(ix = 1; ix < n; ix = ix +1)
        if(min > array[ix]) min = array[ix];
    return(min);
}

/*----------------------------------------------------------
Function: getMax
Parameters:
    array - reference to an array with double values
    n - number of elements in the array
Returns
    max:  the maximum value found in the array
Description: Traverses the array to find its maximum value.
----------------------------------------------------------------*/
double getMax(double *array, double n)
{
    int ix;
    double max = array[0];
    for(ix = 1; ix < n; ix = ix +1)
        if(max < array[ix]) max = array[ix];
    return(max);
}
