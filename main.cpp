#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#define TILE_SIZE 16

enum CellState
{
    NONE,
    WIRE,
    HEAD,
    TAIL
};

sf::View view;

/// \brief Represents a wireworld grid. Responsible for maintaining, updating, and rendering the
/// current state. Also, this representation of wireworld wraps both vertically and horizontally.
class Grid
{
    public:
        Grid(int width, int height) : mWidth(width), mHeight(height), mCells(mWidth*mHeight, Cell{NONE, NONE}),
            mRect(sf::Vector2f(TILE_SIZE, TILE_SIZE))
        {
        }

        ~Grid()
        {
        }

        /// \brief Update the grid
        void update()
        {
            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                int cell = getCell(x, y);

                switch (cell)
                {
                    case WIRE: // wire logic
                    {
                        int neighbors = 0; // Number of neighbor electron heads

                        if (getCell(wrapX(x-1), wrapY(y-1)) == HEAD) neighbors++; // top left
                        if (getCell(x, wrapY(y-1)) == HEAD) neighbors++; // top mid
                        if (getCell(wrapX(x+1), wrapY(y-1)) == HEAD) neighbors++; // top right

                        if (getCell(wrapX(x-1), y) == HEAD) neighbors++; // mid left
                        if (getCell(wrapX(x+1), y) == HEAD) neighbors++; // mid right

                        if (getCell(wrapX(x-1), wrapY(y+1)) == HEAD) neighbors++; // bot left
                        if (getCell(x, wrapY(y+1)) == HEAD) neighbors++; // bot mid
                        if (getCell(wrapX(x+1), wrapY(y+1)) == HEAD) neighbors++; // bot right

                        if (neighbors == 1 || neighbors == 2)
                            setCell(x, y, HEAD); // becomes electron head

                        break;
                    }

                    case HEAD: // electron head logic
                    {
                        setCell(x, y, TAIL);
                        break;
                    }

                    case TAIL: // electron tail logic
                    {
                        setCell(x, y, WIRE);
                        break;
                    }
                }
            }
        }

        /// \brief Draw the grid
        void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default)
        {
            sf::FloatRect viewRect(view.getCenter().x-view.getSize().x/2, view.getCenter().y-view.getSize().y/2, view.getSize().x, view.getSize().y);

            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                if (!viewRect.intersects(sf::FloatRect(x*TILE_SIZE, y*TILE_SIZE, (x+1)*TILE_SIZE, (y+1)*TILE_SIZE)))
                    continue;

                mRect.setPosition(x*TILE_SIZE, y*TILE_SIZE);

                switch (getCell(x, y))
                {
                case NONE: // Air
                    mRect.setFillColor(sf::Color::Black);
                    break;
                case WIRE: // Wire
                    mRect.setFillColor(sf::Color::Yellow);
                    break;
                case HEAD: // Electron head
                    mRect.setFillColor(sf::Color::Blue);
                    break;
                case TAIL: // Electron tail
                    mRect.setFillColor(sf::Color::Red);
                    break;
                }

                target.draw(mRect, states);
            }
        }

        /// \brief Set the next state to the current state
        void flip()
        {
            for (auto& pos : mInteresting)
            {
                int x = pos.x;
                int y = pos.y;

                mCells[y*mWidth + x].current = mCells[y*mWidth + x].next;
            }
        }

        /// \brief Get the contents of a cell
        CellState getCell(int x, int y) const
        {
            return mCells[y*mWidth + x].current;
        }

        /// \brief Set the contents of a cell.
        void setCell(int x, int y, CellState cell)
        {
            if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
                return;

            if (mCells[y*mWidth + x].next == 0 && cell != 0)
                mInteresting.push_back(sf::Vector2i(x, y));
            mCells[y*mWidth + x].next = cell;
        }

        /// \brief Compute the wrapped x coordinate
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

        /// \brief Compute the wrapped y coordinate.
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
            CellState current;
            CellState next;
        };

        int mWidth;
        int mHeight;
        std::vector<Cell> mCells;
        sf::RectangleShape mRect;

        std::vector<sf::Vector2i> mInteresting;
};

int main()
{
    std::cout << "Wireworld Simulator\n";
    std::cout << "Theodore DeRego\n";
    std::cout << "CS 321 @ UH Hilo\n";
    std::cout << "Spring 2014\n\n";

    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 608), "Wireworld Simulator");

    std::ifstream file("primes.wi");
    int width = 0;
    int height = 0;

    // First load grid dims and create grid
    file >> width >> height;
    Grid grid(width, height);

    // Now load the data
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            char c = file.get();
            if (c == ' ')
                grid.setCell(x, y, NONE);
            else if (c == '#')
                grid.setCell(x, y, WIRE);
            else if (c == '@')
                grid.setCell(x, y, HEAD);
            else if (c == '~')
                grid.setCell(x, y, TAIL);
        }
        file.get(); // skip the new line
    }
    file.close();

    sf::Clock clock;
    float dtAccum = 0.f;
    int frames = 0;
    bool paused = false;
    bool render = true;
    while (window.isOpen())
    {
        // Get delta time
        float dt = clock.restart().asSeconds();
        dtAccum += dt;

        if (dtAccum >= 1.f)
        {
            std::cout << frames << std::endl;
            dtAccum = 0.f;
            frames = 0;
        }
        frames++;

        // Grab all of the events!!!
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space)
                {
                    if (event.key.shift)
                        render = !render;
                    else
                        paused = !paused;
                }
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
                grid.setCell(gridX, gridY, TAIL);
            else
                grid.setCell(gridX, gridY, HEAD);
        }

        // Right mouse to place a wire
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            int gridX = mousePos.x/TILE_SIZE;
            int gridY = mousePos.y/TILE_SIZE;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                grid.setCell(gridX, gridY, NONE);
            else
                grid.setCell(gridX, gridY, WIRE);
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

        if (render)
            grid.draw(window);

        // end the current frame
        window.display();
    }

    return 0;
}
