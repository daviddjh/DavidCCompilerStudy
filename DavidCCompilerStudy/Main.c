// DavidCCompilerStudy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdlib.h>
#include <stdio.h>
#include "dinput.h"

int main()
{
    /*
    ii_newfile("Example.txt");
    unsigned char c;
    for (int i = 0; i < 20; i++) {
        c = ii_advance();
        printf("%c", c);
    }
    */
    ii_loadFile("Example.txt");
    char c;
    for (int i = 0; i < 20; i++) {
        c = consumeChar();
        printf("%c", c);
    }
    putbackChar(20);
    printf("\n");
    for (int i = 0; i < 20; i++) {
        c = consumeChar();
        printf("%c", c);
    }
    printf("\n");
    system("pause");
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
