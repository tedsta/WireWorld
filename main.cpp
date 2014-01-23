#include <iostream>

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
            for (int x = 0; x < mWidth; x++)
            {
                for (int y = 0; y < mHeight; y++)
                {
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
        }

        void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default)
        {
            for (int x = 0; x < mWidth; x++)
            {
                for (int y = 0; y < mHeight; y++)
                {
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
        }

        // Set the next state to the current state
        void flip()
        {
            for (int x = 0; x < mWidth; x++)
            {
                for (int y = 0; y < mHeight; y++)
                {
                    mCells[y*mWidth + x].current = mCells[y*mWidth + x].next;
                }
            }
        }

        int getCell(int x, int y) const
        {
            return mCells[y*mWidth + x].current;
        }

        void setCell(int x, int y, int cell)
        {
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
};

int main()
{
    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 608), "Wire World");

    Grid grid(50, 38);
    bool paused = false;

    sf::Clock clock;
    while (window.isOpen())
    {
        // Cap the framerate
        while (clock.getElapsedTime().asSeconds() < 0.016666);
        clock.restart();

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

        grid.flip();

        // clear the window with black color
        window.clear(sf::Color::Black);

        grid.draw(window);

        // end the current frame
        window.display();
    }

    return 0;
}
