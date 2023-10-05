
class Ball
{
public:
    Vector2 position;
    Vector2 lastPosition;
    Vector2 velocity;
    Vector2 acceleration = {0, 0.1};
    float radius;
    Color col = RED;

    Ball(){

    }

    Ball(float x, float y, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->lastPosition.x = x;
        this->lastPosition.y = y;

        // Make rainbow balls gradually shift with globalBalls and a sine function
        int r = (int)(sin(globalBallsCount * 0.01) * 127 + 128);
        int g = (int)(sin(globalBallsCount * 0.01 + 2) * 127 + 128);
        int b = (int)(sin(globalBallsCount * 0.01 + 4) * 127 + 128);
        col = {r, g, b, 255};
    };

    Ball(float x, float y, Vector2 vel, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity = vel;
        this->lastPosition.x = x - vel.x;
        this->lastPosition.y = y - vel.y;
        int r = (int)(sin(globalBallsCount * 0.01) * 127 + 128);
        int g = (int)(sin(globalBallsCount * 0.01 + 2) * 127 + 128);
        int b = (int)(sin(globalBallsCount * 0.01 + 4) * 127 + 128);
        col = {r, g, b, 255};
    };

    void update(float dt)
    {
        // Using dt
        Vector2 displacement = Vector2Subtract(position, lastPosition);
        lastPosition = position;
        position = position + displacement * dragCoef + acceleration * dt * dt;
    }

    void bounds()
    {
        getVelocity();
        if (position.x + radius > maxWidth)
        {
            position.x = maxWidth - radius;
            lastPosition.x = position.x + velocity.x * elasticCoef;
        }
        if (position.x - radius < minWidth)
        {
            position.x = minWidth + radius;
            lastPosition.x = position.x + velocity.x * elasticCoef;
        }
        if (position.y + radius > maxHeight)
        {
            position.y = maxHeight - radius;
            lastPosition.y = position.y + velocity.y * elasticCoef;
        }
        if (position.y - radius < minHeight)
        {
            position.y = minHeight + radius;
            lastPosition.y = position.y + velocity.y * elasticCoef;
        }
    }

    void draw()
    {
        DrawCircle(position.x, position.y, radius, col);
    }

    void collision(int debug);

    void setVelocity(Vector2 v)
    {
        lastPosition = position - v;
    }

    void getVelocity()
    {
        velocity = position - lastPosition;
    }
};

