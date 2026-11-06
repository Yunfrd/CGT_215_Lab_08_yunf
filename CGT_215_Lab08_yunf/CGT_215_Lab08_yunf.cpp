// BalloonBuster.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace sfp;

const float KB_SPEED = 0.5; //crossbow movement speed

void LoadTex(Texture& tex, string filename) { //help to load texture
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}

void MoveCrossbow(PhysicsSprite& crossbow, int elapsedMS) { //moves the crossbow left and right based on what I press on keyboard
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x + (KB_SPEED * elapsedMS); 
        crossbow.setCenter(newPos);
    }
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x - (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
}

Vector2f GetTextSize(Text text) { //get the width and height of sfml text
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    RenderWindow window(VideoMode(800, 600), "Duck Buster"); //size of the window
    World world(Vector2f(0, 0)); //0 gravity so ducks and arrows and fly
    int score(0); // score starting from the zero
    int arrows(5); //amount of arrows

    SoundBuffer popBuffer; //sound effect
    if (!popBuffer.loadFromFile("balloonpop.ogg")) {
        cout << "coul not load balloonpop.ogg" << endl;
        exit(5);
    }
    Sound popSound;
    popSound.setBuffer(popBuffer);

    Music music;
    if (!music.openFromFile("circus.ogg")) {
        cout << "Failed to load circus.ogg ";
        exit(6);
    }
    music.play();

    PhysicsSprite& crossBow = *new PhysicsSprite(); //creates a crossbow
    Texture cbowTex;
    LoadTex(cbowTex, "images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400,
        600 - (sz.y / 2)));

    PhysicsSprite arrow;    //creates an arrow hidden until fire

    Texture arrowTex;
    LoadTex(arrowTex, "images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);

    PhysicsRectangle top; //hidden wall to destroy missed arrows
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);

    PhysicsRectangle right; // hidden wall  to destroy ducks
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    Texture redTex;//load duck texture
    LoadTex(redTex, "images/duck.png");
    PhysicsShapeList<PhysicsSprite> ducks;


    top.onCollision = [&drawingArrow, &world, &arrow]//it destroys arrow when it hits the wall
    (PhysicsBodyCollisionResult result) {
        drawingArrow = false;
        world.RemovePhysicsBody(arrow);
        };

    Font fnt;//loads font for the texts
    if (!fnt.loadFromFile("arial.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }
    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

    long duckMS(0); //timer between duck spawning 
    while ((arrows > 0) || drawingArrow) { // game loops while player has ammo and last fired arrow is still on the air
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();
        if (deltaMS > 9) {
            lastTime = currentTime;
            world.UpdatePhysics(deltaMS);
            MoveCrossbow(crossBow, deltaMS);
            if (Keyboard::isKeyPressed(Keyboard::Space) &&//fires an arrow when space is pressed
                !drawingArrow) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);

            }
            duckMS = duckMS + deltaMS; //increment our duck counter
            if (duckMS > 2000) { //2000 MS is 2 seconds
                duckMS = 0; //reset spawn timer

                PhysicsSprite& duck = ducks.Create(); //new duck
                duck.setTexture(redTex);
                Vector2f sz = duck.getSize();
                duck.setCenter(Vector2f(0, 20 + (sz.y / 2))); //changed x to 0 becaus eit will start from the left coner anyways
                duck.setVelocity(Vector2f(0.25, 0));//fly to the right
                world.AddPhysicsBody(duck);
                duck.onCollision =
                    [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &popSound, &right]
                    (PhysicsBodyCollisionResult result) {
                    if (result.object2 == arrow) {//removes the duck when it gets hit by an arrow
                        popSound.play();
                        drawingArrow = false; 
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                        score += 10; //adds points on the score
                    }
                    else if (result.object2 == right) { //used else if so when duck hits right wall it disappears
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                    }
                };
            }

            window.clear();
            if (drawingArrow) {//render only arrow when its on air
                window.draw(arrow);
            }
            ducks.DoRemovals();
            for (PhysicsShape& duck : ducks) {
                window.draw((PhysicsSprite&)duck);
            }
            window.draw(crossBow); // draw crossbow
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(760,560)); //xhanged the position of the text
            window.draw(scoreText);
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(10, 560));
            window.draw(arrowCountText);
            //world.VisualizeAllBounds(window);

            window.display();

        }
    }
    window.display(); // this is needed to see the last frame
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();
    while (true);

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
