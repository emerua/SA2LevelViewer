#include <glad/glad.h>
#include <cstring>
#include <Windows.h>

#include "../entity.h"
#include "../../toolbox/vector.h"
#include "3spring.h"
#include "../../models/texturedmodel.h"
#include "../../loading/objLoader.h"
#include "../../loading/levelloader.h"
#include "../../main/main.h"
#include "../../collision/collisionmodel.h"
#include "../../collision/collisionchecker.h"
#include "../../toolbox/maths.h"
#include "../dummy.h"
#include "../unknown.h"

#include <list>



std::list<TexturedModel*> THREESPRING::models;
CollisionModel* THREESPRING::cmBase;

THREESPRING::THREESPRING()
{

}

void THREESPRING::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }
    
    despawnGuides();
}

THREESPRING::THREESPRING(char data[32], bool useDefaultValues)
{
    std::memcpy(rawData, data, 32);

    ID = data[1];

    signed short rX;
    signed short rY;
    signed short rZ;

    char* ptr = (char*)(&rX);
    memset(ptr, data[3], 1);
    memset(ptr+1, data[2], 1);

    ptr = (char*)(&rY);
    memset(ptr, data[5], 1);
    memset(ptr+1, data[4], 1);

    ptr = (char*)(&rZ);
    memset(ptr, data[7], 1);
    memset(ptr+1, data[6], 1);

    rotationX = (int)rX;
    rotationY = (int)rY;
    rotationZ = (int)rZ;

    char* x = (char*)&position.x;
    x[3] = data[8];
    x[2] = data[9];
    x[1] = data[10];
    x[0] = data[11];

    char* y = (char*)&position.y;
    y[3] = data[12];
    y[2] = data[13];
    y[1] = data[14];
    y[0] = data[15];

    char* z = (char*)&position.z;
    z[3] = data[16];
    z[2] = data[17];
    z[1] = data[18];
    z[0] = data[19];

    char* v1 = (char*)&power;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];
    power += 5.0f;

    float var2;
    char* v2 = (char*)&var2;
    v2[3] = data[24];
    v2[2] = data[25];
    v2[1] = data[26];
    v2[0] = data[27];
    controlLockTime = (int)var2;

    char* v3 = (char*)&var3;
    v3[3] = data[28];
    v3[2] = data[29];
    v3[1] = data[30];
    v3[0] = data[31];

    if (useDefaultValues)
    {
        rotationX = 0;
        rotationY = 0;
        rotationZ = 0;
        power = 8.0f;
        controlLockTime = 30;
        var3 = 0.0f;
    }

    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    visible = true;
    baseColour.set(1, 1, 1);
    updateTransformationMatrixYXZ();

    collideModelOriginal = THREESPRING::cmBase;
    collideModelTransformed = THREESPRING::cmBase->duplicateMe();
    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);
    updateCollisionModelYXZ();
}

bool THREESPRING::isSA2Object()
{
    return true;
}

void THREESPRING::step()
{
    if (Global::selectedSA2Object == this)
    {
        baseColour.set(1.75f, 1.75f, 1.75f);
    }
    else
    {
        baseColour.set(1.0f, 1.0f, 1.0f);
        if (guides.size() > 0)
        {
            despawnGuides();
            Global::redrawWindow = true;
        }
    }
}

std::list<TexturedModel*>* THREESPRING::getModels()
{
    return &THREESPRING::models;
}

void THREESPRING::loadStaticModels()
{
    if (THREESPRING::models.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading THREESPRING static models...\n");
    #endif

    loadModel(&THREESPRING::models, Global::dirProgRoot + "res/Models/GlobalObjects/Spring/", "TripleSpring");

    if (THREESPRING::cmBase == nullptr)
    {
        THREESPRING::cmBase = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/Spring/", "TripleSpring");
    }
}

void THREESPRING::deleteStaticModels()
{
    #ifdef DEV_MODE
    std::fprintf(stdout, "Deleting THREESPRING static models...\n");
    #endif

    Entity::deleteModels(&THREESPRING::models);
    Entity::deleteCollisionModel(&THREESPRING::cmBase);
}

void THREESPRING::updateValue(int btnIndex)
{
    char buf[128];
    std::string text = buf;

    switch (btnIndex)
    {
    case 0:
    {
        try
        {
            //we are going to change into a new object.
            int newid = std::stoi(text);

            if (newid != ID)
            {
                char data[32] = {0};
                data[ 1] = (char)newid;

                data[ 8] = *(((char*)&position.x)+3);
                data[ 9] = *(((char*)&position.x)+2);
                data[10] = *(((char*)&position.x)+1);
                data[11] = *(((char*)&position.x)+0);
                data[12] = *(((char*)&position.y)+3);
                data[13] = *(((char*)&position.y)+2);
                data[14] = *(((char*)&position.y)+1);
                data[15] = *(((char*)&position.y)+0);
                data[16] = *(((char*)&position.z)+3);
                data[17] = *(((char*)&position.z)+2);
                data[18] = *(((char*)&position.z)+1);
                data[19] = *(((char*)&position.z)+0);

                SA2Object* newObject = LevelLoader::newSA2Object(Global::levelID, newid, data, true);
                if (newObject != nullptr)
                {
                    newObject->lvlLineNum = lvlLineNum;
                    Global::addEntity(newObject);
                    Global::selectedSA2Object = newObject;
                    newObject->updateEditorWindows();
                    Global::redrawWindow = true;
                    cleanUp();
                    Global::deleteEntity(this);
                    return;
                }
            }
            break;
        }
        catch (...) { break; }
    }

    case 2:
    {
        try
        {
            float newX = std::stof(text);
            position.x = newX;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 3:
    {
        try
        {
            float newY = std::stof(text);
            position.y = newY;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 4:
    {
        try
        {
            float newZ = std::stof(text);
            position.z = newZ;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }
    
    case 5:
    {
        try
        {
            int newRotX = std::stoi(text);
            rotationX = newRotX;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 6:
    {
        try
        {
            int newRotY = std::stoi(text);
            rotationY = newRotY;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 7:
    {
        try
        {
            int newRotZ = std::stoi(text);
            rotationZ = newRotZ;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 8:
    {
        try
        {
            float newVar1 = std::stof(text);
            power = newVar1;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 9:
    {
        try
        {
            int newVar2 = std::stoi(text);
            controlLockTime = newVar2;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 10:
    {
        try
        {
            float newVar3 = std::stof(text);
            var3 = newVar3;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    default: break;
    }

    spawnGuides();
}

void THREESPRING::updateEditorWindows()
{




    updateTransformationMatrixYXZ();
    updateCollisionModelYXZ();
    spawnGuides();
}


void THREESPRING::despawnGuides()
{
    for (Dummy* guide : guides)
    {
        Global::deleteEntity(guide);
    }
    guides.clear();
}

void THREESPRING::spawnGuides()
{
    despawnGuides();
    
    Vector3f pos(&position);
    Vector3f dir(0, 10, 0);
    Vector3f xAxis(1, 0, 0);
    Vector3f yAxis(0, 1, 0);
    Vector3f zAxis(0, 0, 1);
    dir = Maths::rotatePoint(&dir, &xAxis, Maths::bamsToRad(rotationX));
    dir = Maths::rotatePoint(&dir, &zAxis, Maths::bamsToRad(rotationZ));

    #ifndef SAB_GUIDES
    for (int i = 0; i < 30; i++)
    {
        Dummy* guide = new Dummy(&Unknown::modelsGuide); INCR_NEW("Entity");
        guide->setPosition(&pos);
        guide->visible = true;
        guide->updateTransformationMatrixYXZ();
        Global::addEntity(guide);
        guides.push_back(guide);
    
        pos = pos + dir;
    }
    #else

    float springPower = fmaxf(100.0f, power*60.0f);

    Vector3f off(0, 5.29734f, 5.88928f);
    off = Maths::rotatePoint(&off, &yAxis, Maths::bamsToRad(rotationY));
    pos = pos + off;

    Vector3f oppositeDir(&off);
    oppositeDir.y = 0;
    oppositeDir.normalize();
    oppositeDir.scale(-1.0f);
    oppositeDir.setLength(5);

    Vector3f vel(&oppositeDir);
    vel.y = springPower;
    float timeLeft = ((float)controlLockTime)/60.0f;
    timeLeft = fmaxf(0.1f, timeLeft);
    const float dt = 0.0166666666666f;
    while (timeLeft > 0)
    {
        Dummy* guide = new Dummy(&Unknown::modelsGuide); INCR_NEW("Entity");
        guide->setPosition(&pos);
        guide->visible = true;
        guide->updateTransformationMatrixYXZ();
        Global::addEntity(guide);
        guides.push_back(guide);
    
        pos = pos + vel.scaleCopy(dt);

        const float airNeutralFriction = 1.25f;
        float storedVelY = vel.y;
        vel.y = 0;
        vel = Maths::applyDrag(&vel, -airNeutralFriction, dt);
        vel.y = storedVelY;

        const float gravityForce = 280.0f;
        const float gravityTerminal = -650.0f;
        const float gravityApproach = 0.45f;
        vel.y = Maths::approach(vel.y, gravityTerminal, gravityApproach, dt);

        timeLeft -= dt;
    }
    #endif
}

void THREESPRING::fillData(char data[32])
{
    data[1] = (char)ID;

    data[2] = (char)((rotationX >> 8) & 0xFF);
    data[3] = (char)((rotationX >> 0) & 0xFF);
    data[4] = (char)((rotationY >> 8) & 0xFF);
    data[5] = (char)((rotationY >> 0) & 0xFF);
    data[6] = (char)((rotationZ >> 8) & 0xFF);
    data[7] = (char)((rotationZ >> 0) & 0xFF);

    char* ptr = (char*)(&position.x);
    data[ 8] = (char)(*(ptr + 3));
    data[ 9] = (char)(*(ptr + 2));
    data[10] = (char)(*(ptr + 1));
    data[11] = (char)(*(ptr + 0));

    ptr = (char*)(&position.y);
    data[12] = (char)(*(ptr + 3));
    data[13] = (char)(*(ptr + 2));
    data[14] = (char)(*(ptr + 1));
    data[15] = (char)(*(ptr + 0));

    ptr = (char*)(&position.z);
    data[16] = (char)(*(ptr + 3));
    data[17] = (char)(*(ptr + 2));
    data[18] = (char)(*(ptr + 1));
    data[19] = (char)(*(ptr + 0));

    float var1 = (power - 5.0f);
    ptr = (char*)(&var1);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));

    float var2 = (float)controlLockTime;
    ptr = (char*)(&var2);
    data[24] = (char)(*(ptr + 3));
    data[25] = (char)(*(ptr + 2));
    data[26] = (char)(*(ptr + 1));
    data[27] = (char)(*(ptr + 0));

    ptr = (char*)(&var3);
    data[28] = (char)(*(ptr + 3));
    data[29] = (char)(*(ptr + 2));
    data[30] = (char)(*(ptr + 1));
    data[31] = (char)(*(ptr + 0));
}
