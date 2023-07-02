#define UNICODE
#include "olcConsoleGameEngine.h"
#include <math.h>
#include <random>
#include <vector>

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f

class AsteroidsGameEngine : public olcConsoleGameEngine {
private:
  struct Vector2D {
    float x;
    float y;

    Vector2D() = default;
    Vector2D(float x, float y) : x(x), y(y) {}
    Vector2D(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}

    Vector2D operator=(const Vector2D &rhs) {
      x = rhs.x;
      y = rhs.y;
      return *this;
    }

    Vector2D operator+(const Vector2D &rhs) const { return Vector2D(x + rhs.x, y + rhs.y); }

    Vector2D operator-(const Vector2D &rhs) const { return Vector2D(x - rhs.x, y - rhs.y); }

    Vector2D operator*(const float &rhs) const { return Vector2D(x * rhs, y * rhs); }

    Vector2D operator/(const float &rhs) const { return Vector2D(x / rhs, y / rhs); }

    Vector2D &operator+=(const Vector2D &rhs) {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    Vector2D &operator-=(const Vector2D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    Vector2D &operator*=(const float &rhs) {
      x *= rhs;
      y *= rhs;
      return *this;
    }

    friend Vector2D operator*(const float &lhs, const Vector2D &rhs) { return rhs * lhs; }

    Vector2D &operator/=(const float &rhs) {
      x /= rhs;
      y /= rhs;
      return *this;
    }

    float magnitude() const { return sqrtf(x * x + y * y); }

    Vector2D normalize() const {
      float mag = magnitude();
      return Vector2D(x / mag, y / mag);
    }

    float getAngle() const { return atan2f(-y, x); }

    void rotate(float angle) {
      // coordinate system is flipped, so negate the angle
      angle = -angle;

      float cosA = cosf(angle);
      float sinA = sinf(angle);
      float tx   = x * cosA - y * sinA;
      float ty   = x * sinA + y * cosA;
      x          = tx;
      y          = ty;
    }
  };

  struct Transform {
    Vector2D pos;
    Vector2D vel;
    int nSize;
    float rotateAngle;
  };

  const float bulletSpeed         = 50.f;
  const float asteroidSpeedMult   = 5.f;
  const float playerConstantSpeed = 2.f;
  const float playerThrust        = 20.f;
  const int asteroidSizeMin       = 8;
  const int asteroidSizeMax       = 30;
  const float astroidSplitSpeed   = 10.f;

  const std::vector<Vector2D> vecModelPlayer{{0.f, -5.5f}, {-2.5f, 2.5f}, {2.5f, 2.5f}};
  const std::vector<Vector2D> vecModelFlame{{-3.f, 4.f}, {-2.f, 6.5f}, {-1.f, 5.f}, {0.f, 6.5f},
                                            {1.f, 5.f},  {2.f, 6.5f},  {3.f, 4.f}};

  std::mt19937 randomEngine{std::random_device{}()};
  std::uniform_real_distribution<float> randomAngle{0.f, TWO_PI};
  std::uniform_real_distribution<float> randomZeroToOne{0.f, 1.f};

  bool isDead;
  bool isIgniting;
  unsigned int score;

  // model of asteroid, dynamically constructed when the game starts
  std::vector<Vector2D> vecModelAstroid;
  // stores the space information of all asteroids
  std::vector<Transform> vecAsteroids;
  // stores the space information of all bullets
  std::vector<Transform> vecBullets;

  // stores the space information of the player
  Transform player;

public:
  AsteroidsGameEngine() : olcConsoleGameEngine() { m_sAppName = L"Asteroids"; }

  void angleToVector(float angle, float mult, Vector2D &vec) {
    vec.x = cosf(angle) * mult;
    vec.y = -sinf(angle) * mult;
  }

  // one time initialization
  void createAsteroidModel() {
    int verts = 20;
    for (int i = 0; i < verts; i++) {
      float radius = 1.f;
      float a      = ((float)i / (float)verts) * TWO_PI;
      Vector2D v{};
      angleToVector(a, radius, v);
      vecModelAstroid.emplace_back(v);
    }
  }

  // reset all dynamic game objects
  void resetGame() {
    vecAsteroids.clear();
    vecBullets.clear();
    isDead = false;
    score  = 0;

    // reset player
    player.pos.x       = ScreenWidth() / 2.f;
    player.pos.y       = ScreenHeight() / 2.f;
    player.vel.x       = 0.f;
    player.vel.y       = 0.f;
    player.rotateAngle = 0.f;

    // create asteroids
    for (int i = 0; i < 5; i++) {
      // determine the speed and direction of the asteroid
      float angle = randomAngle(randomEngine);

      Vector2D asteroidVel;
      float speed = randomZeroToOne(randomEngine) * asteroidSpeedMult;
      angleToVector(angle, speed, asteroidVel);
      asteroidVel.y -= playerConstantSpeed;

      // determine the size of the asteroid
      int size = static_cast<int>(randomZeroToOne(randomEngine) * (asteroidSizeMax - asteroidSizeMin)) + asteroidSizeMin;

      Vector2D asteroidPos;
      asteroidPos.x = static_cast<float>(ScreenWidth() * randomZeroToOne(randomEngine));
      asteroidPos.y = static_cast<float>(ScreenHeight() * randomZeroToOne(randomEngine));

      vecAsteroids.emplace_back(Transform{asteroidPos, asteroidVel, size, 0.f});
    }
  }

  virtual bool OnUserCreate() override {
    createAsteroidModel();
    resetGame();

    return true;
  }

  virtual bool OnUserUpdate(float fElapsedTime) override {
    Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

    // control player
    // steer
    if (m_keys[VK_LEFT].bHeld || m_keys['A'].bHeld)
      player.rotateAngle += 5.f * fElapsedTime;
    if (m_keys[VK_RIGHT].bHeld || m_keys['D'].bHeld)
      player.rotateAngle -= 5.f * fElapsedTime;
    // thrust
    if (m_keys[VK_UP].bHeld || m_keys['W'].bHeld) {
      Vector2D acc;
      angleToVector(player.rotateAngle + PI / 2, playerThrust, acc);
      player.vel += acc * fElapsedTime;
      isIgniting = true;
    } else {
      isIgniting = false;
    }

    player.pos += player.vel * fElapsedTime;
    WrapCoordinates(player.pos);

    // draw bullets
    if (m_keys[VK_SPACE].bPressed) {
      Vector2D localBulletPos{0.f, -5.5f};
      localBulletPos.rotate(player.rotateAngle);

      Vector2D localBulletVel{};
      angleToVector(player.rotateAngle + PI / 2, bulletSpeed, localBulletVel);
      vecBullets.emplace_back(Transform{localBulletPos + player.pos, localBulletVel + player.vel, 0, 0.f});
    }

    // update and draw all asteroids
    int loopSize = vecAsteroids.size();
    for (int i = 0; i < loopSize; i++) {
      auto &a = vecAsteroids[i];
      a.pos += a.vel * fElapsedTime;
      WrapCoordinates(a.pos);

      DrawWireframeModel(vecModelAstroid, a.pos, a.rotateAngle, a.nSize, FG_YELLOW);

      for (int j = i + 1; j < loopSize; j++) {
        auto &a2 = vecAsteroids[j];
        if (IsCirclesCollided(a.pos, a.nSize, a2.pos, a2.nSize)) {
          int smallerAsteroidIndex = a.nSize < a2.nSize ? i : j;
          int largerAsteroidIndex  = a.nSize < a2.nSize ? j : i;

          auto &smallerAsteroid = vecAsteroids[smallerAsteroidIndex];
          auto &largerAsteroid  = vecAsteroids[largerAsteroidIndex];
          largerAsteroid.vel +=
              (static_cast<float>(smallerAsteroid.nSize) / largerAsteroid.nSize) * (smallerAsteroid.vel - largerAsteroid.vel);
          vecAsteroids.erase(vecAsteroids.begin() + smallerAsteroidIndex);
          loopSize--;
        }
      }
    }

    // update and draw all bullets
    for (auto &b : vecBullets) {
      b.pos += b.vel * fElapsedTime;
      Draw(b.pos.x, b.pos.y);

      // check for collision with asteroids
      int checkSize = vecAsteroids.size();
      for (int i = 0; i < checkSize; i++) {
        auto &a = vecAsteroids[i];
        // asteroid hit
        if (IsPointInsideCircle(b.pos, a.pos, a.nSize)) {
          // remove bullet
          b.pos.x = -100;

          // split asteroid
          if (a.nSize >= asteroidSizeMin) {
            Vector2D v1, v2;
            Vector2D offset1, offset2;

            float divAngle = b.vel.getAngle();
            float angle1   = divAngle + 0.5f * PI;
            float angle2   = divAngle - 0.5f * PI;
            angleToVector(angle1, astroidSplitSpeed, v1);
            angleToVector(angle2, astroidSplitSpeed, v2);
            angleToVector(angle1, a.nSize / 2. + 1, offset1);
            angleToVector(angle2, a.nSize / 2. + 1, offset2);
            vecAsteroids.emplace_back(Transform{a.pos + offset1, v1 + a.vel, static_cast<int>(a.nSize / 2. + 1), 0.f});
            vecAsteroids.emplace_back(Transform{a.pos + offset2, v2 + a.vel, static_cast<int>(a.nSize / 2. + 1), 0.f});
          }

          vecAsteroids.erase(vecAsteroids.begin() + i);
          // avoid checking with newly added asteroids
          checkSize--;
          // we only check collision with the first asteroid hit
          break;
        }
      }
    }

    // remove bullets that are off screen
    if (vecBullets.size()) {
      auto i = std::remove_if(vecBullets.begin(), vecBullets.end(), [this](const Transform &b) {
        return (b.pos.x < 0 || b.pos.x >= ScreenWidth() || b.pos.y < 0 || b.pos.y >= ScreenHeight());
      });
      if (i != vecBullets.end())
        vecBullets.erase(i, vecBullets.end());
    }

    // draw player
    DrawWireframeModel(vecModelPlayer, player.pos, player.rotateAngle, 1., FG_CYAN);
    if (isIgniting)
      DrawWireframeModel(vecModelFlame, player.pos, player.rotateAngle, 1., FG_RED);
    return true;
  }

  virtual bool OnUserDestroy() override { return true; }

  // overloaded draw function to wrap coordinates
  virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F) override {
    Vector2D wrapped{x, y};
    WrapCoordinates(wrapped);
    olcConsoleGameEngine::Draw(wrapped.x, wrapped.y, c, col);
  }

  // wrap coordinates to screen size
  void WrapCoordinates(Vector2D &v) {
    if (v.x < 0.f)
      v.x += (float)ScreenWidth();
    if (v.x >= (float)ScreenWidth())
      v.x -= (float)ScreenWidth();
    if (v.y < 0.f)
      v.y += (float)ScreenHeight();
    if (v.y >= (float)ScreenHeight())
      v.y -= (float)ScreenHeight();
  }

  // draw a wireframe model
  void DrawWireframeModel(const std::vector<Vector2D> &vecModelCoord, Vector2D offset, float angle, float scale = 1,
                          int col = FG_WHITE) {
    std::vector<Vector2D> transformedCoords{};

    // rotation, scaling and translation
    for (const auto &vec : vecModelCoord) {
      Vector2D transformedVec = vec;
      transformedVec.rotate(angle);
      transformedCoords.emplace_back(transformedVec * scale + offset);
    }

    for (int i = 0; i < transformedCoords.size(); i++) {
      int j = (i + 1) % transformedCoords.size();
      // 0-1, 1-2 ... and wrap around
      DrawLine(transformedCoords[i].x, transformedCoords[i].y, transformedCoords[j].x, transformedCoords[j].y, PIXEL_SOLID, col);
    }
  }

  // check if a point is inside a circle
  bool IsPointInsideCircle(Vector2D p, Vector2D o, float radius) { return IsCirclesCollided(p, 0.f, o, radius); }

  bool IsCirclesCollided(Vector2D o1, float r1, Vector2D o2, float r2) {
    float dx        = o1.x - o2.x;
    float dy        = o1.y - o2.y;
    float fDistance = sqrtf(dx * dx + dy * dy);
    return fDistance < r1 + r2;
  }
};

int main() {
  AsteroidsGameEngine asteroidsGameEngine{};
  asteroidsGameEngine.ConstructConsole(160, 100, 8, 8);
  asteroidsGameEngine.Start();

  system("pause");
  return 0;
}
