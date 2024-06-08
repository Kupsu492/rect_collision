#include <stdio.h>
#include <math.h>

#include "raylib.h"

typedef struct Game {
    float screenWidth;
    float screenHeight;

    Rectangle *rects;
    int rectCount;
} Game;

typedef struct PlayerPetal {
    Rectangle rect;
    float speed;
} PlayerPetal;

typedef enum RectSide {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
} RectSide;

typedef struct BallRectCollision {
    Vector2 pos;
    float near;
    RectSide side;
} BallRectCollision;

void swapCoordinates(float *a, float *b) {
    float t = *a;
    *a = *b;
    *b = t;
}

// typedef struct Rectangle {
//     float x;                // Rectangle top-left corner position x
//     float y;                // Rectangle top-left corner position y
//     float width;            // Rectangle width
//     float height;           // Rectangle height
// } Rectangle;

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
// bool CheckBallRectCollision(Vector2 oldPos, Vector2 newPos, float radius, Rectangle rec)
// {
//     bool collision = false;

//     float recCenterX = rec.x + rec.width/2.0f;
//     float recCenterY = rec.y + rec.height/2.0f;

//     float dx = fabsf(center.x - recCenterX);
//     float dy = fabsf(center.y - recCenterY);

//     if (dx > (rec.width/2.0f + radius)) { return false; }
//     if (dy > (rec.height/2.0f + radius)) { return false; }

//     if (dx <= (rec.width/2.0f)) { return true; }
//     if (dy <= (rec.height/2.0f)) { return true; }

//     float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
//                              (dy - rec.height/2.0f)*(dy - rec.height/2.0f);

//     collision = (cornerDistanceSq <= (radius*radius));

//     return collision;
// }

void movePetal(Game *gameInfo, PlayerPetal *petal, float dir) {
    float yPos = petal->rect.y + petal->speed * dir * GetFrameTime();
    if (yPos < 0) {
        petal->rect.y = 0;
    } else if (yPos > (gameInfo->screenHeight - petal->rect.height)) {
        petal->rect.y = gameInfo->screenHeight - petal->rect.height;
    } else {
        petal->rect.y = yPos;
    }
}

bool rayTraceCollide(Vector2 startPoint, Vector2 dir, Rectangle rect, Vector2 *hit, float *hitNear, RectSide *wallHit) {
    Vector2 near, far;

    // Get time value of rect x y axis on direction ray
    near.x = (rect.y - startPoint.y) / dir.y;
    near.y = (rect.x - startPoint.x) / dir.x;
    far.x =  (rect.y + rect.height - startPoint.y) / dir.y;
    far.y =  (rect.x + rect.width - startPoint.x) / dir.x;

    // Sort time values so that near has smaller lengths
    if (near.x > far.x) swapCoordinates(&near.x, &far.x);
    if (near.y > far.y) swapCoordinates(&near.y, &far.y);

    // First collision check to controll that ray can go through the rect
    if (near.x > far.y || near.y > far.x) return false;

    *hitNear = (near.x > near.y) ? near.x : near.y;
    float hitFar = (far.x < far.y) ? far.x : far.y;

    // Check that collision doesen't happen behind or past the ray
    // In other words, that collision happens between the start and end points
    if (hitFar < 0 || (* hitNear) > 1) return false;

    hit->x = startPoint.x + (* hitNear) * dir.x;
    hit->y = startPoint.y + (* hitNear) * dir.y;

    // Note: if near.x == near.y then ray hit rect in corner.
    // if near.y > near.x then we can check top and bottom hit
    if (near.x > near.y) {
        *wallHit = (dir.x < 0) ? BOTTOM : TOP;
    } else {
        *wallHit = (dir.y < 0) ? LEFT : RIGHT;
    }

    return true;
}

void getBallPos(Vector2 *pos, Vector2 *startPoint, Vector2 *dir, float unitSpot) {
    Vector2 dirUnit = (Vector2) {
        dir->x > 0 ? 1 : -1,
        dir->y > 0 ? 1 : -1
    };
    Vector2 tVector = (Vector2) {
        ((dir->y * unitSpot) - 15 * dirUnit.y) / dir->y,
        ((dir->x * unitSpot) - 15 * dirUnit.x) / dir->x
    };

    float t = (tVector.x > tVector.y) ? tVector.x : tVector.y;

    pos->x = startPoint->x + t * dir->x;
    pos->y = startPoint->y + t * dir->y;
}

void getBallPosWithCollision(Vector2 *pos, Vector2 *startPoint, Vector2 *dir, Rectangle rect, float hit, RectSide wallHit) {
    float movDir;

    Vector2 cornerPoint;
    float t, cornerYDistance, xDist;
    if (wallHit == TOP || wallHit == BOTTOM) {
        // movDir = (dir->x > 0) ? 1 : -1;
        // cornerT = (cornerPoint - startPoint->x) / dir->x;
        movDir = (dir->x > 0) ? 1 : -1;
        cornerPoint.x = (dir->x > 0) ? rect.x : rect.x + rect.width;
        t = ((cornerPoint.x - startPoint->x)) / dir->x;
        // pos->x = startPoint->x + t * dir->x;
        // pos->y = startPoint->y + t * dir->y;
        // return;

        cornerPoint.y = startPoint->y + t * dir->y;
        cornerYDistance = rect.y - cornerPoint.y; //TODO: check if other corner
        if (cornerYDistance < 15 && cornerYDistance > 0) {
            xDist = sqrt(fabs(cornerYDistance*cornerYDistance - 15*15));
            t = ((cornerPoint.x - xDist - startPoint->x)) / dir->x;
        } else {
            // If past the corner
            movDir = (dir->y > 0) ? 1 : -1;
            t = ((dir->y * hit) - 15 * movDir) / dir->y;
        }

    } else {
        movDir = (dir->x > 0) ? 1 : -1;
        t = ((dir->x * hit) - 15 * movDir) / dir->x;
    }

    pos->x = startPoint->x + t * dir->x;
    pos->y = startPoint->y + t * dir->y;
}

int main(void)
{
    Game gameInfo;

    gameInfo.screenWidth = 800.0f;
    gameInfo.screenHeight = 450.0f;

    InitWindow(gameInfo.screenWidth, gameInfo.screenHeight, "My raylib game");

    PlayerPetal petal = {
        { gameInfo.screenWidth/2, gameInfo.screenHeight/2, 100, 100 },
        500
    };

    int testPointCount = 0;
    Vector2 testPoints[3] = {
        {gameInfo.screenWidth/2 - 200, 100},
        {gameInfo.screenWidth/2 + 250, 400},
        {gameInfo.screenWidth/2 + 250, gameInfo.screenHeight/2}
    };

    Vector2 startPoint = testPoints[0];
    Vector2 endPoint, ballPos, rayDir, hitPos;
    float hitNear;
    RectSide wallHit;

    Color rectColor, lineColor;
    bool rectCollide;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Player movement
        if (IsKeyDown(KEY_J)) movePetal(&gameInfo, &petal, 1);
        if (IsKeyDown(KEY_K)) movePetal(&gameInfo, &petal, -1);

        if (IsKeyPressed(KEY_L)) {
            startPoint = testPoints[++testPointCount % 3];
        }

        endPoint = GetMousePosition();
        rayDir = (Vector2) {
            endPoint.x - startPoint.x,
            endPoint.y - startPoint.y
        };

        rectCollide = rayTraceCollide(startPoint, rayDir, petal.rect, &hitPos, &hitNear, &wallHit);

        if (rectCollide) {
            rectColor = PURPLE;
            lineColor = RED;
            getBallPosWithCollision(&ballPos, &startPoint, &rayDir, petal.rect, hitNear, wallHit);
        } else {
            rectColor = BLUE;
            lineColor = GREEN;
            getBallPos(&ballPos, &startPoint, &rayDir, 1);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawRectangleRec(petal.rect, rectColor);
            DrawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, lineColor);

            if (rectCollide) {
                // DrawCircleV(hitPos, 5, MAROON);
            }
            DrawCircleLinesV(ballPos, 15, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
