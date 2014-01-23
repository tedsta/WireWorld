#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#define TILE_SIZE 16

class Grid
{
    public:
        Grid(int width, int height) : mWidth(width), mHeight(height), mCells(mWidth*mHeight, Cell{0, 0}),
            mRect(sf::Vector2f(TILE_SIZE, TILE_SIZE))
        {
        }

        ~Grid()
        {
        }

        void update()
        {
            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                int cell = getCell(x, y);

                switch (cell)
                {
                    case 1: // wire logic
                    {
                        int neighbors = 0; // Number of neighbor electron heads

                        if (getCell(wrapX(x-1), wrapY(y-1)) == 2) neighbors++; // top left
                        if (getCell(x, wrapY(y-1)) == 2) neighbors++; // top mid
                        if (getCell(wrapX(x+1), wrapY(y-1)) == 2) neighbors++; // top right

                        if (getCell(wrapX(x-1), y) == 2) neighbors++; // mid left
                        if (getCell(wrapX(x+1), y) == 2) neighbors++; // mid right

                        if (getCell(wrapX(x-1), wrapY(y+1)) == 2) neighbors++; // bot left
                        if (getCell(x, wrapY(y+1)) == 2) neighbors++; // bot mid
                        if (getCell(wrapX(x+1), wrapY(y+1)) == 2) neighbors++; // bot right

                        if (neighbors == 1 || neighbors == 2)
                            setCell(x, y, 2); // becomes electron head

                        break;
                    }

                    case 2: // electron head logic
                    {
                        setCell(x, y, 3);
                        break;
                    }

                    case 3: // electron tail logic
                    {
                        setCell(x, y, 1);
                        break;
                    }
                }
            }
        }

        void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default)
        {
            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                mRect.setPosition(x*TILE_SIZE, y*TILE_SIZE);

                switch (getCell(x, y))
                {
                case 0: // Air
                    mRect.setFillColor(sf::Color::Black);
                    break;
                case 1: // Wire
                    mRect.setFillColor(sf::Color::Yellow);
                    break;
                case 2: // Electron head
                    mRect.setFillColor(sf::Color::Blue);
                    break;
                case 3: // Electron tail
                    mRect.setFillColor(sf::Color::Red);
                    break;
                }

                target.draw(mRect, states);
            }
        }

        // Set the next state to the current state
        void flip()
        {
            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                mCells[y*mWidth + x].current = mCells[y*mWidth + x].next;
            }
        }

        int getCell(int x, int y) const
        {
            return mCells[y*mWidth + x].current;
        }

        void setCell(int x, int y, int cell)
        {
            if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
                return;

            if (mCells[y*mWidth + x].next == 0 && cell != 0)
                mInteresting.push_back(sf::Vector2i(x, y));
            mCells[y*mWidth + x].next = cell;
        }

        int wrapX(int x) const
        {
            if (x < 0)
            {
                x = x % mWidth;
                if (x < 0)
                    x += mWidth;
            }
            else if (x >= mWidth)
                x = x % mWidth;

            return x;
        }

        int wrapY(int y) const
        {
            if (y < 0)
            {
                y = y % mHeight;
                if (y < 0)
                    y += mHeight;
            }
            else if (y >= mHeight)
                y = y % mHeight;

            return y;
        }

    private:
        struct Cell
        {
            int current;
            int next;
        };

        int mWidth;
        int mHeight;
        std::vector<Cell> mCells;
        sf::RectangleShape mRect;

        std::vector<sf::Vector2i> mInteresting;
};

int main()
{
    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 608), "Wire World");

    std::ifstream file("primes.wi");
    int width = 0;
    int height = 0;

    // First load grid dims and create grid
    file >> width >> height;
    Grid grid(width, height);
    bool paused = false;

    // Now load the data
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            char c = file.get();
            if (c == ' ')
                grid.setCell(x, y, 0);
            else if (c == '#')
                grid.setCell(x, y, 1);
            else if (c == '@')
                grid.setCell(x, y, 2);
            else if (c == '~')
                grid.setCell(x, y, 3);
        }
        file.get(); // skip the new line
    }
    file.close();

    sf::Clock clock;
    sf::View view;
    while (window.isOpen())
    {
        // Get delta time
        float dt = clock.restart().asSeconds();

        // Grab all of the events!!!
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space)
                    paused = !paused;
            }
        }

        if (!paused)
            grid.update();

        // Left mouse to place an electron head
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            int gridX = mousePos.x/TILE_SIZE;
            int gridY = mousePos.y/TILE_SIZE;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                grid.setCell(gridX, gridY, 3);
            else
                grid.setCell(gridX, gridY, 2);
        }

        // Right mouse to place a wire
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            int gridX = mousePos.x/TILE_SIZE;
            int gridY = mousePos.y/TILE_SIZE;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                grid.setCell(gridX, gridY, 0);
            else
                grid.setCell(gridX, gridY, 1);
        }

        // Move the camera
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            view.move(0, -1000.f*dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            view.move(0, 1000.f*dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            view.move(-1000.f*dt, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            view.move(1000.f*dt, 0);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            view.zoom(1.f+dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
            view.zoom(1.f-dt);

        grid.flip();

        // clear the window with black color
        window.setView(view);
        window.clear(sf::Color::Black);

        grid.draw(window);

        // end the current frame
        window.display();
    }

    return 0;
}
