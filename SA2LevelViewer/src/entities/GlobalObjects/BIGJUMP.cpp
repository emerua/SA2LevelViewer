#include <glad/glad.h>
#include <cstring>
#include <Windows.h>

#include "../entity.h"
#include "../../toolbox/vector.h"
#include "bigjump.h"
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



std::list<TexturedModel*> BIGJUMP::models;
CollisionModel* BIGJUMP::cmBase;

BIGJUMP::BIGJUMP()
{

}

void BIGJUMP::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }
    
    despawnGuides();
}

BIGJUMP::BIGJUMP(char data[32], bool useDefaultValues)
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

    char* v1 = (char*)&powerH;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];

    float var2;
    char* v2 = (char*)&var2;
    v2[3] = data[24];
    v2[2] = data[25];
    v2[1] = data[26];
    v2[0] = data[27];
    cooldown = (int)var2;

    char* v3 = (char*)&powerV;
    v3[3] = data[28];
    v3[2] = data[29];
    v3[1] = data[30];
    v3[0] = data[31];
    powerV += 3.2f;

    if (useDefaultValues)
    {
        rotationX = 0;
        rotationY = 0;
        rotationZ = 0;
        powerH = 7.0f;
        cooldown = 40;
        powerV = 5.0f;
    }

    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    visible = true;
    baseColour.set(1, 1, 1);

    collideModelOriginal = BIGJUMP::cmBase;
    collideModelTransformed = BIGJUMP::cmBase->duplicateMe();
    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);

    #ifndef SAB_GUIDES
    updateTransformationMatrixYXZ();
    updateCollisionModelYXZ();
    #else
    updateTransformationMatrixXY();
    updateCollisionModelXY();
    #endif
}

bool BIGJUMP::isSA2Object()
{
    return true;
}

void BIGJUMP::step()
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

std::list<TexturedModel*>* BIGJUMP::getModels()
{
    return &BIGJUMP::models;
}

void BIGJUMP::loadStaticModels()
{
    if (BIGJUMP::models.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading BIGJUMP static models...\n");
    #endif

    loadModel(&BIGJUMP::models, Global::dirProgRoot + "res/Models/GlobalObjects/BigJump/", "BigJump");
    if (BIGJUMP::cmBase == nullptr)
    {
        BIGJUMP::cmBase = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/BigJump/", "BigJump");
    }
}

void BIGJUMP::deleteStaticModels()
{
    #ifdef DEV_MODE
    std::fprintf(stdout, "Deleting BIGJUMP static models...\n");
    #endif

    Entity::deleteModels(&BIGJUMP::models);
    Entity::deleteCollisionModel(&BIGJUMP::cmBase);
}

void BIGJUMP::updateValue(int btnIndex)
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
            powerH = newVar1;
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
            cooldown = newVar2;
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
            powerV = newVar3;
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    default: break;
    }

    #ifndef SAB_GUIDES
    updateTransformationMatrixYXZ();
    updateCollisionModelYXZ();
    #else
    updateTransformationMatrixXY();
    updateCollisionModelXY();
    #endif

    spawnGuides();
}

void BIGJUMP::updateEditorWindows()
{



    #ifndef SAB_GUIDES
    #else
    #endif

    #ifndef SAB_GUIDES
    updateTransformationMatrixYXZ();
    updateCollisionModelYXZ();
    #else
    updateTransformationMatrixXY();
    updateCollisionModelXY();
    #endif

    spawnGuides();
}

void BIGJUMP::despawnGuides()
{
    for (Dummy* guide : guides)
    {
        Global::deleteEntity(guide);
    }
    guides.clear();
}

void BIGJUMP::spawnGuides()
{
    despawnGuides();

    #ifndef SAB_GUIDES
    const float GRAVITY = -0.08f; //TODO hard coded value of gravity

    Vector3f pos(&position);
    Vector3f deltaH(0, 0, -powerH);
    Vector3f yAxis(0, 1, 0);
    deltaH = Maths::rotatePoint(&deltaH, &yAxis, Maths::bamsToRad(rotationY));
    float speedV = powerV+GRAVITY;
    int count = 0;

    bool debug = false;
    if (cooldown == 31 && powerH == 1.6f && powerV == 4.6f)
    {
        debug = true;
    }

    if (debug)
    {
        //printf("deltaH = %f, %f, %f\n", deltaH.x, deltaH.y, deltaH.z);
    }


    for (int i = 0; i < 200; i++) //TODO hard coded value for number of frames
    {
        Dummy* guide = new Dummy(&Unknown::modelsGuide); INCR_NEW("Entity");
        guide->setPosition(&pos);
        guide->visible = true;
        guide->updateTransformationMatrixYXZ();
        Global::addEntity(guide);
        guides.push_back(guide);

        pos = pos + deltaH;
        pos.y += speedV;

        count++;

        //Vector3f diffH = pos-position;
        //diffH.y = 0;
        //if (diffH.length() > 50.0f || count > cooldown/2)
        //{
        //    speedV += GRAVITY;
        //}

        Vector3f diff = pos-position;
        //33 might be variable. 20 is hard coded
        if (diff.length() > 33.0f || count > 20)
        {
            speedV += GRAVITY;
        }

        if (debug)
        {
            //printf("speedV = %f\n", speedV);
        }

        if (pos.y < position.y)
        {
            break;
        }
    }

    //printf("done\n\n");

    #else
    Vector3f pos(&position);
    pos.y += 4;

    Vector3f dir(0, 0, -1);
    Vector3f xAxis(1, 0, 0);
    Vector3f yAxis(0, 1, 0);
    dir = Maths::rotatePoint(&dir, &xAxis, Maths::toRadians(20.0f)); //hard coded 20 degree offset
    dir = Maths::rotatePoint(&dir, &xAxis, Maths::bamsToRad(rotationX)); //additional rot from var
    dir = Maths::rotatePoint(&dir, &yAxis, Maths::bamsToRad(rotationY));

    Vector3f vel = dir;
    vel.setLength(powerH*60.0f);
    float timeLeft = ((float)cooldown)/60.0f;
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

void BIGJUMP::fillData(char data[32])
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

    ptr = (char*)(&powerH);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));

    float var2 = (float)cooldown;
    ptr = (char*)(&var2);
    data[24] = (char)(*(ptr + 3));
    data[25] = (char)(*(ptr + 2));
    data[26] = (char)(*(ptr + 1));
    data[27] = (char)(*(ptr + 0));

    float var3 = powerV - 3.2f;
    ptr = (char*)(&var3);
    data[28] = (char)(*(ptr + 3));
    data[29] = (char)(*(ptr + 2));
    data[30] = (char)(*(ptr + 1));
    data[31] = (char)(*(ptr + 0));
}
