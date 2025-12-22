#include <Windows.h>   
#include <iostream>    
#include <cstdlib>     
#include <ctime>       

using namespace std;




const int TRACK_LENGTH = 50;


const int TURTLE_COUNT = 5;





struct Turtle
{
    int id;        
    int position;  
};




Turtle turtles[TURTLE_COUNT];


char track[TURTLE_COUNT][TRACK_LENGTH + 1];


bool finished[TURTLE_COUNT] = {false};


int winner = -1;


CRITICAL_SECTION cs;


int LocalRandom(unsigned int& seed,int l)
{
    seed = seed * 1103515245 + 12345;
    return  0+(seed%(3 - 0 + 1));
}

void UpdateTrack()
{
    
    for (int i = 0; i < TURTLE_COUNT; i++)
    {

        for (int j = 0; j < TRACK_LENGTH; j++)
            track[i][j] = '_';


        track[i][TRACK_LENGTH] = '\0';


        int pos = turtles[i].position;


        if (pos >= TRACK_LENGTH)
            pos = TRACK_LENGTH - 1;


        track[i][pos] = 'T';
    }
}


void DrawScreen()
{

    system("cls");
    cout << "ГОНКА ЧЕРЕПАШЕК\n\n";
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        cout << "Черепашка " << turtles[i].id << ": |"
            << track[i] << "|\n";
    }
    cout << "\n--- СОБЫТИЯ ---\n";
    for (int i = 0; i < TURTLE_COUNT; i++)
        if (finished[i])
        {
            cout << "Черепашка" << turtles[i].id << "дошла до финиша\n";
        }
}

bool allfin(bool t[])
{
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        if (!t[i])
            return false;
    }
    return true;
}


DWORD WINAPI TurtleRun(LPVOID param)
{
    
    Turtle* turtle = (Turtle*)param;

    unsigned int seed = ((unsigned int)time(NULL)+GetTickCount()+turtle->id+100);
    

    
    while (true)
    {
        if (turtle->position >= TRACK_LENGTH)
            break;
       
        int step = LocalRandom(seed,turtle->id);

        
        turtle->position += step;

        
        if (turtle->position > TRACK_LENGTH)
            turtle->position = TRACK_LENGTH;

        
        
        
        if (turtle->position >= TRACK_LENGTH && !finished[turtle->id - 1])
        {
            finished[turtle->id - 1] = true;
            
            if (winner == -1)
                winner = turtle->id;
        }
        
        UpdateTrack();
      

       


       


        Sleep(LocalRandom(seed,turtle->id)+1*1000+turtle->id*10);
    }


    return 0;
}


int main()
{
    
    setlocale(LC_ALL, "ru");
    srand((int)time(NULL));

    
   

    HANDLE threads[TURTLE_COUNT];

    
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        turtles[i].id = i + 1;
        turtles[i].position = 0;
    }

    
    UpdateTrack();

    
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        threads[i] = CreateThread(
            NULL,               
            0,                  
            TurtleRun,          
            &turtles[i],        
            0,                  
            NULL                
        );
    }

    bool t = false;
    while (!t)
    {
        DrawScreen();
        t = allfin(finished);
        Sleep(1000);
    }
    
    WaitForMultipleObjects(
        TURTLE_COUNT,
        threads,
        TRUE,
        INFINITE
    );

    

    if (allfin(finished))
    {
        cout<<"Победила "<<winner<<" черепашка" << endl;
    }
    
    for (int i = 0; i < TURTLE_COUNT; i++)
        CloseHandle(threads[i]);

    
    

    system("pause");
    return 0;
}