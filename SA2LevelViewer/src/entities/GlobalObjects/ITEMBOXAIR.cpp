#include <glad/glad.h>
#include <cstring>
#include <Windows.h>

#include "../entity.h"
#include "../../toolbox/vector.h"
#include "itemboxair.h"
#include "../../models/texturedmodel.h"
#include "../../loading/objLoader.h"
#include "../../loading/levelloader.h"
#include "../../main/main.h"
#include "../../collision/collisionmodel.h"
#include "../../collision/collisionchecker.h"
#include "../../toolbox/maths.h"
#include "../../toolbox/hex.h"
#include "../dummy.h"

#include <list>
#include <vector>


std::list<TexturedModel*> ITEMBOXAIR::modelsBase;
std::list<TexturedModel*> ITEMBOXAIR::modelsShell;
std::vector<std::list<TexturedModel*>> ITEMBOXAIR::modelsItem;

CollisionModel* ITEMBOXAIR::cmBaseBase = nullptr;
CollisionModel* ITEMBOXAIR::cmBaseShell = nullptr;

ITEMBOXAIR::ITEMBOXAIR()
{

}

void ITEMBOXAIR::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }
    
    if (collideModelShellTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelShellTransformed);
        collideModelShellTransformed = nullptr;
    }
    
    if (shell != nullptr)
    {
        Global::deleteTransparentEntity(shell);
        shell = nullptr;
    }
    
    if (item != nullptr)
    {
        Global::deleteEntity(item);
        item = nullptr;
    }
}

ITEMBOXAIR::ITEMBOXAIR(char data[32], bool useDefaultValues)
{
    std::memcpy(rawData, data, 32);

    ID = data[1];

    signed short rX = 0;
    signed short rY = 0;
    signed short rZ = 0;

    char* ptr = (char*)(&rX);
    memset(ptr, data[3], 1);
    memset(ptr+1, data[2], 1);

    ptr = (char*)(&rY);
    memset(ptr, data[5], 1);
    memset(ptr+1, data[4], 1);

    ptr = (char*)(&rZ);
    memset(ptr, data[7], 1);
    memset(ptr+1, data[6], 1);

    rotationX = (int)(rX);
    rotationY = (int)(rY);
    rotationZ = (int)(rZ);

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

    float var1;
    char* v1 = (char*)&var1;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];

    char* v2 = (char*)&var2;
    v2[3] = data[24];
    v2[2] = data[25];
    v2[1] = data[26];
    v2[0] = data[27];

    char* v3 = (char*)&var3;
    v3[3] = data[28];
    v3[2] = data[29];
    v3[1] = data[30];
    v3[0] = data[31];
    
    itemType = (int)var1;

    if (useDefaultValues)
    {
        itemType = 1;
        var2 = 0.0f;
        var3 = 0.0f;
        rotationX = 0;
        rotationY = 0;
        rotationZ = 0; 
    }

    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    visible = true;
    updateTransformationMatrixZY();

    collideModelOriginal         = ITEMBOXAIR::cmBaseBase;
    collideModelTransformed      = ITEMBOXAIR::cmBaseBase->duplicateMe();
    collideModelShellTransformed = ITEMBOXAIR::cmBaseShell->duplicateMe();

    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);
    updateCollisionModelZY();

    collideModelShellTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelShellTransformed);
    updateCollisionModelZY(ITEMBOXAIR::cmBaseShell, collideModelShellTransformed);

    shell = new Dummy(&ITEMBOXAIR::modelsShell); INCR_NEW("Entity");
    shell->setPosition(&position);
    shell->setRotation(rotationX, rotationY, rotationZ);
    shell->visible = true;
    shell->baseColour.set(1.0f, 1.0f, 1.0f);
    shell->updateTransformationMatrixZY();
    Global::addTransparentEntity(shell);

    item = new Dummy(&ITEMBOXAIR::modelsItem[std::min(10, std::max(0, itemType))]); INCR_NEW("Entity");
    item->setPosition(&position);
    item->setRotation(rotationX, rotationY, rotationZ);
    item->setScale(1.75f, 1.75f, 1.75f);
    item->visible = true;
    item->baseColour.set(1.0f, 1.0f, 1.0f);
    item->updateTransformationMatrixZY();
    Global::addEntity(item);
}

bool ITEMBOXAIR::isSA2Object()
{
    return true;
}

void ITEMBOXAIR::step()
{
    if (Global::selectedSA2Object == this)
    {
        baseColour.set(1.6f, 1.6f, 1.6f);
        shell->baseColour.set(1.6f, 1.6f, 1.6f);
    }
    else
    {
        baseColour.set(1.0f, 1.0f, 1.0f);
        shell->baseColour.set(1.0f, 1.0f, 1.0f);
    }
}

std::list<TexturedModel*>* ITEMBOXAIR::getModels()
{
    return &ITEMBOXAIR::modelsBase;
}

void ITEMBOXAIR::loadStaticModels()
{
    if (ITEMBOXAIR::modelsBase.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading ITEMBOXAIR static models...\n");
    #endif

    loadModel(&ITEMBOXAIR::modelsBase,  Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/", "ItemBoxAirBase");
    loadModel(&ITEMBOXAIR::modelsShell, Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/", "ItemBoxAirShell");

    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemSpeedUp");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemRing5");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "Item1Up");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemRing10");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemRing20");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemBarrierG");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemBomb");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemHealth");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemBarrierB");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemNothing");
    ITEMBOXAIR::loadNewItemModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/Items/", "ItemInvincible");

    if (ITEMBOXAIR::cmBaseBase == nullptr)
    {
        ITEMBOXAIR::cmBaseBase = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/", "ItemBoxAirBase");
    }
    if (ITEMBOXAIR::cmBaseShell == nullptr)
    {
        ITEMBOXAIR::cmBaseShell = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/ItemBox/", "ItemBoxAirShell");
    }
}

void ITEMBOXAIR::loadNewItemModel(std::string folder, std::string filename)
{
    std::list<TexturedModel*> item;
    loadModel(&item, folder, filename);
    ITEMBOXAIR::modelsItem.push_back(item);
}

void ITEMBOXAIR::deleteStaticModels()
{
    #ifdef DEV_MODE
    std::fprintf(stdout, "Deleting ITEMBOXAIR static models...\n");
    #endif

    Entity::deleteModels(&ITEMBOXAIR::modelsBase);
    Entity::deleteModels(&ITEMBOXAIR::modelsShell);
    for (int i = 0; i < (int)ITEMBOXAIR::modelsItem.size(); i++)
    {
        Entity::deleteModels(&ITEMBOXAIR::modelsItem[i]);
    }
    ITEMBOXAIR::modelsItem.clear();
    Entity::deleteCollisionModel(&ITEMBOXAIR::cmBaseBase);
    Entity::deleteCollisionModel(&ITEMBOXAIR::cmBaseShell);
}

void ITEMBOXAIR::updateValue(int btnIndex)
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
        }
        catch (...) { }
        break;
    }

    case 2:
    {
        try
        {
            float newX = std::stof(text);
            position.x = newX;
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
            break;
        }
        catch (...) { break; }
    }

    case 5:
    {
        try
        {
            short newRotX = Hex::stohshort(text);
            rotationX = (int)newRotX;
            break;
        }
        catch (...) { break; }
    }

    case 6:
    {
        try
        {
            short newRotY = Hex::stohshort(text);
            rotationY = (int)newRotY;
            break;
        }
        catch (...) { break; }
    }

    case 7:
    {
        try
        {
            short newRotZ = Hex::stohshort(text);
            rotationZ = (int)newRotZ;
            break;
        }
        catch (...) { break; }
    }

    case 8:
    {
        try
        {
            float newVar1 = std::stof(text);
            itemType = (int)newVar1;
            break;
        }
        catch (...) { break; }
    }

    case 9:
    {
        try
        {
            float newVar2 = std::stof(text);
            var2 = newVar2;
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
            break;
        }
        catch (...) { break; }
    }
    
    default: break;
    }

    Global::redrawWindow = true;

    shell->setPosition(&position);
    shell->setRotation(rotationX, rotationY, rotationZ);
    item->setPosition(&position);
    item->setScale(1.75f, 1.75f, 1.75f);
    item->setRotation(rotationX, rotationY, rotationZ);

    item->setModels(&ITEMBOXAIR::modelsItem[std::min(10, std::max(0, itemType))]);

    updateTransformationMatrixZY();
    shell->updateTransformationMatrixZY();
    item->updateTransformationMatrixZY();

    updateCollisionModelZY();
    updateCollisionModelZY(ITEMBOXAIR::cmBaseShell, collideModelShellTransformed);
}

void ITEMBOXAIR::updateEditorWindows()
{




    shell->setPosition(&position);
    shell->setRotation(rotationX, rotationY, rotationZ);
    item->setPosition(&position);
    item->setScale(1.75f, 1.75f, 1.75f);
    item->setRotation(rotationX, rotationY, rotationZ);

    item->setModels(&ITEMBOXAIR::modelsItem[std::min(10, std::max(0, itemType))]);

    updateTransformationMatrixZY();
    shell->updateTransformationMatrixZY();
    item->updateTransformationMatrixZY();

    updateCollisionModelZY();
    updateCollisionModelZY(ITEMBOXAIR::cmBaseShell, collideModelShellTransformed);
}

void ITEMBOXAIR::fillData(char data[32])
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

    float var1 = (float)itemType;
    ptr = (char*)(&var1);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));

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
