/*
The MIT License (MIT)
Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "MicroBit.h"
#include "DigitalOut.h"

#define SNAKE_HEAD_PIXEL_BRIGHTNESS     150
#define SNAKE_BONE_PIXEL_BRIGHTNESS     15
#define SNAKE_FOOD_PIXEL_BRIGHTNESS     255

template <class T> class Node{
public:
    Node(T data): d(data), next(NULL), prev(NULL) {}
    
    Node * getNext(){
        return next;
    }
    Node * getPrev(){
        return prev;
    }
    void detach () {
        if (prev){
            prev->next = next;
        }
        if (next){
            next->prev = prev;
        }
        
        next = NULL;
        prev = NULL;
    } 
    void append(Node * node) {
        Node * n = this;
        while (n->getNext()){
            n = n->getNext();
        }
        n->next = node;
        if (node){
            node->prev = n;
        }
    }
public:
    T d;
private:
    Node * next;
    Node * prev;
};

template <class T> class List {
public:
    List(): head(NULL), tail(NULL){
    }
    void prepend(Node<T> * node){
        if (node){
            if (head == NULL && tail == NULL) {
                head = tail = node;
            } else {
                node->append(head);
                head = node;
            }
        }
    }
    Node<T> * removeTail(){
        Node<T> * node = tail;
        if (tail) {
            tail = tail->getPrev();
            node->detach();
            if (tail == NULL){
                head = NULL;
            }
        }
        return node;
    }   
    Node<T> * getHead(){
        return head;
    }
    Node<T> * getTail(){
        return tail;
    } 
    void cleanup(){
        Node<T> * node = tail;
        while(node){
            Node<T> * temp = node;
            node = node->getPrev();
            
            temp->detach();
            delete temp;
        }
        head = tail = NULL;
    }
private:
    Node<T> *   head;
    Node<T> *   tail;
};

class Dimension
{
public:
    Dimension (int start, int end) {
        this->start = start;
        this->end = end;
        this->cur = start;
    }
    
    Dimension (int start, int end, int cur) {
        this->start = start;
        this->end = end;
        this->cur = cur;
    }
    
    void operator ++(int){
        cur++;
        if (cur > end){
            cur = start;
        }
    }
    
    void operator --(int){
        cur--;
        if (cur < start){
            cur = end;
        }
    }
    
    int operator -(int i){
        int r = cur - i;
        if (r < start){
            r = end;
        }
        return r;
    }
    
    operator int(){
        return cur;
    }
private:
    int start;
    int end;
    int cur;
};


class SnakeBone {
public:
    SnakeBone(int startx, int endx, int curx, int starty, int endy, int cury):
        x(startx, endx, curx), y(starty, endy, cury)
        {
        }
        
    Dimension getX(){
        return x;
    }
    
    Dimension getY(){
        return y;
    }

private:
    Dimension x;
    Dimension y;
};


class Snake {
public:
    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    Snake (): d(UP){
        bones.prepend(new Node<SnakeBone>(SnakeBone(0, 4, 2, 0, 4, 2)));
    }
    
    Node<SnakeBone> * getHead(){
        return bones.getHead();
    }
    Dimension getHeadX(){
        return _getNodeX(bones.getHead());
    }
    Dimension getHeadY(){
        return _getNodeY(bones.getHead());
    }
    Dimension getTailX(){
        return _getNodeX(bones.getTail());
    }
    Dimension getTailY(){
        return _getNodeY(bones.getTail());
    }
    
    void grow(){
        // Find next position to place new head
        Dimension nextX = getHeadX();
        Dimension nextY = getHeadY();
        
        switch (d){
            case UP: {
                nextY--;
            }
            break;
            case DOWN: {
                nextY++;
            }
            break;
            case LEFT: {
                nextX--;
            }
            break;
            case RIGHT: {
                nextX++;
            }
            break;
        }
        
        // New bone at head 
        bones.prepend(new Node<SnakeBone>(SnakeBone(0, 4, nextX, 0, 4, nextY)));            
    }
    
    void reduce(){
        Node<SnakeBone> * tail = bones.removeTail();
        if (tail){
            delete tail;
        }
    }
    
    void left(){
        switch (d){
            case UP:
            case DOWN: {
                d = LEFT;
            }
            break;
            case LEFT:  {
                d = DOWN;
            }
            break;
            case RIGHT: {
                d = UP;
            }
            break;
        }
    }
    
    void right(){
        switch (d){
            case UP:
            case DOWN: {
                d = RIGHT;
            }
            break;
            case LEFT: {
                d = UP;
            }
            break;
            case RIGHT: {
                d = DOWN;
            }
            break;
        }
    }
    
    void reset(){
        bones.cleanup();
        bones.prepend(new Node<SnakeBone>(SnakeBone(0, 4, 2, 0, 4, 2)));
        d = UP;
    }
    
    Direction getDirection(){
        return d;
    }
    
private:
    Dimension _getNodeX(Node<SnakeBone> * node){
        if (node){
            return node->d.getX();
        }
        return Dimension(-1, -1, -1);
    }
    
    Dimension _getNodeY(Node<SnakeBone> * node){
        if (node){
            return node->d.getY();
        }
        return Dimension(-1, -1, -1);
    }

    List<SnakeBone> bones;
    Direction d;
};

class Game {
public:
    Game(MicroBit & ubit): uBit(ubit), foodx(-1), foody(-1), image(5,5), score(0),
        buttonPressed(false) {
        uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
        uBit.display.print(image);
    }
    
    void left(){
        if (!buttonPressed){
            buttonPressed = true;
            snake.left();
        }
    }
    
    void right(){
        if (!buttonPressed){
            buttonPressed = true;
            snake.right();
        }
    }
    
    bool move(){
        // Dim old head
        Dimension x = snake.getHeadX();
        Dimension y = snake.getHeadY();
        image.setPixelValue(x, y, SNAKE_BONE_PIXEL_BRIGHTNESS);
        
        snake.grow();
        
        Dimension nextX = snake.getHeadX();
        Dimension nextY = snake.getHeadY();
        
        // check if we grew by eating.
        if (nextX == foodx && nextY == foody){
            // food gone
            foodx = -1;
            foody = -1;
            score++;
            
            // Since we have grown to the head. no need to delete tail.
        } else {
            // Switch off end.
            int endx = snake.getTailX();
            int endy = snake.getTailY();
            if (endx != -1 && endy != -1){
                image.setPixelValue(endx, endy, 0);
            }
            snake.reduce();
        }
        
        // Check snake bite
        if (image.getPixelValue(nextX, nextY) == 15){
            // Game Over
            return false;
        }   

        // Turn On head.
        image.setPixelValue(nextX, nextY, SNAKE_HEAD_PIXEL_BRIGHTNESS);
        uBit.display.print(image);
        buttonPressed = false;
        return true;
    }
    
    bool isGoodFood(int x, int y){
        bool ret = image.getPixelValue(x, y) == 0;
        
        // also don't accept food in the direction of movement
        if (ret){
            switch(snake.getDirection()){
                case Snake::UP:
                case Snake::DOWN: {
                    ret = x != snake.getHeadX(); 
                }
                break;
                case Snake::LEFT:
                case Snake::RIGHT: {
                    ret = y != snake.getHeadY();
                }
                break;
            }
        }
        return ret;
    }
    
    void play(){
        while (true) {
            if (!move()){
                showGameOver();
                break;
            }
            
            // Put food
            if (foodx == -1 && foody == -1){
                while (true){
                    int x = uBit.random(5);
                    int y = uBit.random(5);
                    
                    if (isGoodFood(x, y)){
                        foodx = x;
                        foody = y;
                        image.setPixelValue(foodx, foody, 255);
                        uBit.display.print(image);
                        break;
                    }
                }
            }
            uBit.sleep(500);
        }
    }
    
    void animateSnake(){
#define ANIMATION_SPEED 50
        // UP 2
        foodx = 2;foody = 1;
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        foodx = 2;foody = 0;
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        // LEFT
        foodx = 1;foody = 0;
        left();
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        // DOWN
        foodx = 1;foody = 1;
        left();
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        foodx = 1;foody = 2;
        move();
        uBit.sleep(ANIMATION_SPEED);
        move();
        uBit.sleep(ANIMATION_SPEED);
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        for (int i = 0; i < 4; i++){
            // LEFT
            left();
            move();
            uBit.sleep(ANIMATION_SPEED);
            
            // UP
            right();
            move();
            uBit.sleep(ANIMATION_SPEED);
            
            move();
            uBit.sleep(ANIMATION_SPEED);
            move();
            uBit.sleep(ANIMATION_SPEED);
            move();
            uBit.sleep(ANIMATION_SPEED);
            
            // LEFT
            left();
            move();
            uBit.sleep(ANIMATION_SPEED);
            
            // DOWN
            left();
            move();
            uBit.sleep(ANIMATION_SPEED);
            
            move();
            uBit.sleep(ANIMATION_SPEED);
            move();
            uBit.sleep(ANIMATION_SPEED);
            move();
            uBit.sleep(ANIMATION_SPEED);
        }
        
        // Back to the centre
        left();
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        left();
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        move();
        uBit.sleep(ANIMATION_SPEED);
        move();
        uBit.sleep(ANIMATION_SPEED);
        
        // eat the tail
        while(snake.getHead()){
            image.setPixelValue(snake.getTailX(), snake.getTailY(), 0);
            uBit.display.print(image);
            snake.reduce();
            uBit.sleep(ANIMATION_SPEED);
        }
        
        reset();
    }
    
    void showGameOver(){
        // switch off food
        if (foodx != -1 && foody != -1){
            image.setPixelValue(foodx, foody, 0);
            uBit.display.print(image);
        }
        
        // change brightness to 255 as pixels with custom brightness
        // are not affected by display.setBrightness(). Possible Bug!!!
        Node<SnakeBone> * bone = snake.getHead();
        while (bone){
            image.setPixelValue(bone->d.getX(), bone->d.getY(), SNAKE_FOOD_PIXEL_BRIGHTNESS);
            bone = bone->getNext();
        }
        // Flash snake
        uBit.display.print(image);
        bool toggle = false;
        for (int i = 0; i < 10; i++, toggle = !toggle){
            if (toggle)
                uBit.display.setBrightness(255);
            else
                uBit.display.setBrightness(SNAKE_BONE_PIXEL_BRIGHTNESS);
            uBit.sleep(500);
        }
        uBit.display.print(MicroBitImage(5,5));
        uBit.display.setBrightness(255);
        uBit.display.scroll("SCORE-"); // don't mind spaces in a scrolling display.
        uBit.display.scroll(score, 150);
    }
    
    void reset(){
        // cleanup snake and state
        snake.reset();
        
        foodx = -1;
        foody = -1;
        score = 0;
        image = MicroBitImage(5, 5);
        uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
        image.setPixelValue(snake.getHeadX(), snake.getHeadY(), SNAKE_HEAD_PIXEL_BRIGHTNESS);
        uBit.display.print(image);
    }
    
    
private:
    MicroBit & uBit;
    int foodx;
    int foody;
    Snake snake;
    MicroBitImage image;
    int score;
    
    // Sync button press and movement. Don't let quick button presses
    // change snake direction twice/thrice/... before moving. As a 
    // double press can reverse the snake on its own and Game Over!!!
    bool buttonPressed; 
};


MicroBit uBit;
Game game(uBit);


void onButtonA(MicroBitEvent){
    game.left();
}

void onButtonB(MicroBitEvent){
    game.right();
}


int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

    // Continous play    
    while (true){
        game.animateSnake();
        game.play();
        game.reset();
    }
}