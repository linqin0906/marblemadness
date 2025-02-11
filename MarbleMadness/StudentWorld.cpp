#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>

#include "Level.h"
#include "Actor.h"
#include <algorithm>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    player = nullptr;
    levelBonus = 1000;
    numCrystals = 0;
    isLevelComplete = false;
}

int StudentWorld::init()
{
    numCrystals = 0; //needs to be here bc loadALevel adds crystals

    int levelLoad = loadALevel("level0" + to_string(getLevel()) + ".txt");
    
    if (levelLoad == -1 || getLevel() == 100) return GWSTATUS_PLAYER_WON; //no file or finished lvl99
    if (levelLoad == -2) return GWSTATUS_LEVEL_ERROR;
    
    levelBonus = 1000;
    isLevelComplete = false;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    setDisplayText();
    
    bool result = everyoneDoSomething();
    if (!result) {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    } else {player->doSomething();}
    
    cleanDeadActors();
    if (levelBonus != 0) levelBonus--;
    
    if (isLevelComplete) {
        increaseScore(levelBonus);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    if (player != nullptr) delete player; //player isn't already deleted
    player = nullptr;
    
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        Actor* temp = *it;
        it = actorList.erase(it);
        delete temp;
    }
}

//checks for NONPLAYER actors. COULD RETURN ITSELF
Actor* StudentWorld::getActor(int r, int c, Actor* a) {
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        if ((*it)->getX() == r && (*it)->getY() == c && ((*it) != a) && (*it)->isVisible()) //make sure you don't return yourself
            return *it;
        it++;
    }
    return nullptr;
}

//version to check not equal to two other actors
Actor* StudentWorld::getActor(int r, int c, Actor* a, Actor* b) {
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        if ((*it)->getX() == r && (*it)->getY() == c && ((*it) != a) && ((*it) != b) && (*it)->isVisible()) //make sure you don't return yourself
            return *it;
        it++;
    }
    return nullptr;
}

Actor* StudentWorld::getThiefBot(int r, int c) {
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        if ((*it)->getX() == r && (*it)->getY() == c && (*it)->canSteal())
            return *it;
        it++;
    }
    return nullptr;
}

bool StudentWorld::isPlayerOn(int r, int c) {
    if (player == nullptr || !player->isAlive()) return false;
    if (player->getX() == r && player->getY() == c) return true;
    return false;
}

Avatar* StudentWorld::getPlayer() {
    return player;
}

int StudentWorld::getCrystals() {
    return numCrystals;
}

void StudentWorld::decCrystals() {
    numCrystals--;
}

void StudentWorld::setLevelComplete(bool complete) {
    isLevelComplete = complete;
}

void StudentWorld::setDisplayText() {
    
    ostringstream oss;
    oss << "Score: ";
    oss.fill('0');
    oss << setw(7) << getScore();
    oss << "  Level: " << setw(2) << getLevel();
    oss.fill(' ');
    oss << "  Lives: " << setw(2) << getLives();
    oss << "  Health: " << setw(3) << player->getHealthPercentage() << "%";
    oss << "  Ammo: " << setw(3) << player->getPeaCount();
    oss << "  Bonus: " << setw(4) << levelBonus;
    
    string s = oss.str();
    setGameStatText(s);
}

void StudentWorld::spawnPea(int dir, int x, int y) {
    actorList.push_front(new Pea(dir, x, y, this));
}

void StudentWorld::spawnThiefBot(int type, int x, int y) {
    if (type == 1) {
        actorList.push_front(new ThiefBot(x, y, this));
    } else {
        actorList.push_front(new MeanThiefBot(x, y, this));
    }
}

int StudentWorld::computeTicks() {
    int ticks = (28 - getLevel())/4; // level number (0, 1, 2, etc.)
    if (ticks < 3) ticks = 3;
    return ticks;
}

StudentWorld::~StudentWorld() {
    if (player != nullptr) delete player; //player isn't already deleted
    player = nullptr;
    
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        Actor* temp = *it;
        it = actorList.erase(it);
        delete temp;
    }
}

int StudentWorld::loadALevel(string currLevel) {
   
    Level lev(assetPath()); 
    Level::LoadResult result = lev.loadLevel(currLevel);
    
    if (result == Level::load_fail_file_not_found) return -1;
    if (result == Level::load_fail_bad_format) return -2;
        
    for (int r = 0; r < VIEW_WIDTH; r++) {
        for (int c = 0; c < VIEW_HEIGHT; c++) {
            Level::MazeEntry item = lev.getContentsOf(r, c);
            
            if (item == Level::player)
                player = new Avatar(r, c, this);
            else if (item == Level::wall)
                actorList.push_back(new Wall(r, c, this));
            else if (item == Level::crystal) {
                actorList.push_back(new Crystal(r, c, this));
                numCrystals++;
            } else if (item == Level::exit)
                actorList.push_back(new Exit(r, c, this));
            else if (item == Level::extra_life)
                actorList.push_back(new ExtraLife(r, c, this));
            else if (item == Level::restore_health)
                actorList.push_back(new RestoreHealth(r, c, this));
            else if (item == Level::ammo)
                actorList.push_back(new Ammo(r, c, this));
            else if (item == Level::marble)
                actorList.push_back(new Marble(r, c, this));
            else if (item == Level::pit)
                actorList.push_back(new Pit(r, c, this));
            else if (item == Level::horiz_ragebot)
                actorList.push_back(new RageBot(0, r, c, this));
            else if (item == Level::vert_ragebot)
                actorList.push_back(new RageBot(270, r, c, this));
            else if (item == Level::thiefbot_factory       )
                actorList.push_back(new ThiefbotFactory(1, r, c, this));
            else if (item == Level::mean_thiefbot_factory)
                actorList.push_back(new ThiefbotFactory(2, r, c, this));
        }
    }
    return 0;
}

//everyone does something EXCEPT player
bool StudentWorld::everyoneDoSomething() {
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        if ((*it)->isAlive()) {
            (*it)->doSomething();
            if (!player->isAlive()) return false;
        }
        it++;
    }
    return true;
}


void StudentWorld::cleanDeadActors() {
    list<Actor*>::iterator it = actorList.begin();
    while (it != actorList.end()) {
        if (!(*it)->isAlive()) {
            Actor* temp = *it;
            it = actorList.erase(it);
            delete temp;
        }
        else {it++;}
    }
}
