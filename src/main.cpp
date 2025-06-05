#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <nlohmann/json.hpp>
#include "configurations.h"
#include "Stroke.h"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <vector>
#include <queue>
#include <stack>
#include <map>
using json = nlohmann::json;
using ScoreMap = std::map<std::string, int>;
using MergeMap = std::map<std::string, ScoreMap>;
using MuchPlatformMap = std::map<std::string, MergeMap>;

int popCount = 0;
float velocityY = 0.f;         // current vertical velocity
float posGround;
float platformBottom;
bool isOnGround = false;       // track if player is on the ground
bool stateJustChanged = false;
bool wPressedLastFrame = false;
bool sPressedLastFrame = false;
bool aPressedLastFrame = false;
bool dPressedLastFrame = false;
bool enterPressedLastFrame = false;
bool escapePressedLastFrame = false;

int muchPlatform = 30;
bool mergePlatform = false;

enum class GameStates { Menu, SetDif, Score, Playing, Pause, winScreen, loseScreen };
enum class InputEvent { Up, Down, Enter, Left, Right };
std::stack<GameStates> states;

struct Platform 
{
    sf::RectangleShape shape;
    bool spikes = false;
    bool finishLine = false;
};

sf::Clock gameClock;
float pauseTime = 0.f;
bool isPaused = false;
float scores;

float getElapsedTime();

// Saat pause
void pauseClock();

// Saat resume
void resumeClock();

// Initialize platforms somewhere in your setup code:
// @brief input buffer
std::queue<InputEvent> inputBuffer;
void handleInputBuffer();
void clearInputBuffer();

void appendScoresToJson(const std::string& filename, 
                        const std::string& muchplatformKey, 
                        const std::string& mergeKey, 
                        const std::map<std::string, int>& newScores);

void generatePlatforms(std::vector<Platform>& platforms, float startX, float startY, int count, std::mt19937& rng, int spikeStreak = 0, int groundStreak = 0);
void ShuffleRNG(std::mt19937& rng);

void Pause(sf::RenderWindow& window, std::vector<sf::Text> texts, sf::RectangleShape& box);
void goPause();

void menu(sf::RenderWindow& window, sf::Sprite& sprite, sf::RectangleShape* box, int& pos, std::size_t count);
void setDif(sf::RenderWindow &window, std::vector<sf::Text> &texts, std::vector<sf::RectangleShape> &box, sf::Sprite& player, std::vector<Platform>& platforms, std::mt19937& rng, sf::Clock& clock);
void score(sf::RenderWindow& window);

void game(sf::RenderWindow& window, sf::View& cam, sf::Sprite& player, std::vector<Platform>& platforms, float deltaTime);
void winScreen(sf::RenderWindow& window, std::vector<sf::Text>& texts, sf::RectangleShape bg, sf::String input);

int main()
{
    // -----------> Starting Program <-------------
    // @brief Window
    std::cout << "Begin Analyzing";
    auto window = sf::RenderWindow(sf::VideoMode({ conf::window_size.x, conf::window_size.y }), "CMake SFML Project", sf::State::Fullscreen);
    window.setFramerateLimit(conf::max_FPS);

    // @brief Font
    sf::Font default_font;
    if (!default_font.openFromFile("src/fonts/SpaceMono-Regular.ttf"))
    {  // Use forward slashes or raw string literal
        std::cerr << "Failed to load font 'fonts/SpaceMono-Regular.ttf'" << std::endl;
        return -1; // Exit or handle error appropriately
    }

    // @brief Texture
    sf::Texture texture("src/textures/origbig.png"); // Dont Touch
    sf::Texture Menu_Title("src/textures/SAMPLE_Title.png"); // Dont Touch
    sf::Texture Menu_Scoreboard("src/textures/SAMPLE_Scoreboard.png"); // Change for different Biome
    sf::Texture Menu_Start("src/textures/SAMPLE_Start.png"); // Change for different Biome
    sf::Texture Menu_Exit("src/textures/SAMPLE_Exit.png"); // Change for different Biome
    sf::Texture dif_BG("src/textures/Dif Background.png"); // Change for different Biome
    sf::Texture Win_BG("src/textures/Win Background.jpeg"); // Change for different Biome
    sf::Texture optFrame("src/textures/optFrame.png");
    texture.setSmooth(true);

    // @brief Camera
    sf::View view({ window.getView() });
    view.move({ 0.f, 400.f });

    // @brief Add zoom
    float zoomFactor = 0.5f;  // Adjust zoom level
    view.zoom(zoomFactor);

    // @brief Clock
    sf::Clock clock;

    // @brief Platforms Vector's
    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<Platform> platforms;

    // @brief Player
    sf::Texture testTexture("src/textures/Sample_Player.png");
    sf::Sprite player(testTexture);
    player.setOrigin({ player.getLocalBounds().size.x / 2.f, player.getLocalBounds().size.y });
    player.setPosition(conf::resetPos);
    player.setScale({ 3.f, 3.f });

    std::cout << ".";
    // <----------- Starting Program ------------->

    // ----------->  Making Programs <-------------
    std::cout << ".";
    int menu_pos = 1;
    // =========== MENU ===========
    // ---> Menu <---
    // Background Image
    sf::RectangleShape bg({ (float)conf::window_size.x, (float)conf::window_size.y });

    sf::Sprite sprite(texture);
    sprite.setScale
    (
        {
            (float)window.getSize().x / texture.getSize().x,
            (float)window.getSize().y / texture.getSize().y
        }
    );
    // <--- End --->

    // ---> Boxes <---
    
    // box 0 - titles
    sf::RectangleShape menuBox[5];
    menuBox[0].setSize({ 625.f, 250.f });
    menuBox[0].setOrigin({ menuBox[0].getSize().x / 2 , menuBox[0].getSize().y / 2 });
    menuBox[0].setPosition({ conf::window_size_f.x / 2, 175.f});
    menuBox[0].setTexture(&Menu_Title);

    // box 1 - start
    menuBox[1].setSize({ 300.f, 150.f });
    menuBox[1].setOrigin({ menuBox[1].getSize().x / 2 , menuBox[1].getSize().y / 2 });
    menuBox[1].setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 2 + 50.f});
    menuBox[1].setTexture(&Menu_Start);

    // box 2 - scoreboard
    menuBox[2].setSize({ 300.f, 150.f });
    menuBox[2].setOrigin({ menuBox[2].getSize().x / 2 , menuBox[2].getSize().y / 2 });
    menuBox[2].setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 3 + 50.f});
    menuBox[2].setTexture(&Menu_Scoreboard);

    //box 3 - exit
    menuBox[3].setSize({ 300.f, 150.f });
    menuBox[3].setOrigin({ menuBox[2].getSize().x / 2 , menuBox[2].getSize().y / 2 });
    menuBox[3].setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 4 + 50.f});
    menuBox[3].setTexture(&Menu_Exit);

    //box 4 - navigate
    menuBox[4].setSize({ 350.f, 200.f });
    menuBox[4].setOrigin({ menuBox[4].getSize().x / 2 , menuBox[4].getSize().y / 2 });
    menuBox[4].setPosition({ conf::window_size_f.x/2, menuBox[1].getPosition().y});
    menuBox[4].setFillColor(sf::Color(0,0,0,100));

    // <--- End --->
    std::cout << ".";
    // =========== MENU ===========

    // ======= STARTING GAME ======
    // ---> background <---
    sf::Text stopwatchText( conf::font );
    stopwatchText.setCharacterSize( 30 );
    stopwatchText.setString("Time : ");
    stopwatchText.setFillColor( sf::Color::Black );
    stopwatchText.setPosition({ 20.f, 20.f});

    // ---> Tree <---
    sf::Text text(conf::font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::Red);
    text.setPosition({ 10.f, 10.f });
    // <--- Tree --->
    std::cout << ".\n";
    // ======= STARTING GAME ======

    // =======  PAUSING GAME ======
    std::vector<sf::Text> pause_text;
    
    // Title
    sf::Text Pause_Text1(conf::font);
    Pause_Text1.setCharacterSize(80);
    Pause_Text1.setFillColor(sf::Color::Red);
    Pause_Text1.setString("PAUSED GAME");
    Pause_Text1.setOrigin({ Pause_Text1.getGlobalBounds().getCenter().x, Pause_Text1.getGlobalBounds().getCenter().y });
    Pause_Text1.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 });
    pause_text.push_back(Pause_Text1);

    // OPT 1
    sf::Text Pause_Text2(conf::font);
    Pause_Text2.setCharacterSize(55);
    Pause_Text2.setFillColor(sf::Color::Black);
    Pause_Text2.setString("RESUME");
    Pause_Text2.setOrigin({ Pause_Text2.getGlobalBounds().getCenter().x, Pause_Text2.getGlobalBounds().getCenter().y });
    Pause_Text2.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 2 + 50.f });
    pause_text.push_back(Pause_Text2);

    // OPT 2
    sf::Text Pause_Text3(conf::font);
    Pause_Text3.setCharacterSize(55);
    Pause_Text3.setFillColor(sf::Color::Black);
    Pause_Text3.setString("MENU");
    Pause_Text3.setOrigin({ Pause_Text3.getGlobalBounds().getCenter().x, Pause_Text3.getGlobalBounds().getCenter().y });
    Pause_Text3.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 3 + 50.f });
    pause_text.push_back(Pause_Text3);

    // OPT 3
    sf::Text Pause_Text4(conf::font);
    Pause_Text4.setCharacterSize(55);
    Pause_Text4.setFillColor(sf::Color::Red);
    Pause_Text4.setString("QUIT");
    Pause_Text4.setOrigin({ Pause_Text4.getGlobalBounds().getCenter().x, Pause_Text4.getGlobalBounds().getCenter().y });
    Pause_Text4.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 4 + 50.f });
    pause_text.push_back(Pause_Text4);

    // OPT BOX
    sf::RectangleShape pause_box;
    pause_box.setSize({ 300.f, 100.f });
    pause_box.setOrigin({ pause_box.getGlobalBounds().getCenter().x, pause_box.getGlobalBounds().getCenter().y });
    pause_box.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 5 * 2 + 50.f });
    pause_box.setFillColor(sf::Color(191, 191, 191));
    // =======  PAUSING GAME ======

    // =======  SETTING DIFF ======
    std::vector<sf::Text> setDifText;
    std::vector<sf::RectangleShape> setDifBox;

    // Sub 1 and option 1 [0]
    sf::Text difSub1Opt1 (conf::playerFont);
    difSub1Opt1.setString("YES");
    difSub1Opt1.setCharacterSize(80);
    difSub1Opt1.setFillColor(sf::Color::White);
    difSub1Opt1.setOrigin({ difSub1Opt1.getLocalBounds().getCenter().x, difSub1Opt1.getLocalBounds().getCenter().y });
    difSub1Opt1.setPosition({ conf::window_size_f.x / 3 + 50.f, conf::window_size_f.y / 6 * 3 + 10.f });
    setDifText.push_back(difSub1Opt1);

    // Sub 1 and option 2 [1]
    sf::Text difSub1Opt2 (conf::playerFont);
    difSub1Opt2.setString("NO");
    difSub1Opt2.setCharacterSize(80);
    difSub1Opt2.setFillColor(sf::Color::White);
    difSub1Opt2.setOrigin({ difSub1Opt2.getLocalBounds().getCenter().x, difSub1Opt2.getLocalBounds().getCenter().y });
    difSub1Opt2.setPosition({ conf::window_size_f.x / 3 * 2 - 50.f, conf::window_size_f.y / 6 * 3 + 10.f });
    setDifText.push_back(difSub1Opt2);

    // Sub 1 and option 1 [2]
    sf::Text difSub2Opt (conf::playerFont);
    difSub2Opt.setString("20");
    difSub2Opt.setCharacterSize(120);
    difSub2Opt.setFillColor(sf::Color::White);
    difSub2Opt.setOrigin({ difSub2Opt.getLocalBounds().getCenter().x, difSub2Opt.getLocalBounds().getCenter().y });
    difSub2Opt.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 6 * 5 });
    setDifText.push_back(difSub2Opt);

    // [0]
    sf::RectangleShape difBG(conf::window_size_f);
    difBG.setTexture(&dif_BG);
    setDifBox.push_back(difBG);

    // [1]
    sf::RectangleShape sub1({190, 130});
    sub1.setOrigin({sub1.getLocalBounds().getCenter().x, sub1.getLocalBounds().getCenter().y});
    sub1.setPosition(difSub2Opt.getPosition());
    sub1.setTexture(&optFrame);
    sub1.setFillColor(sf::Color(0,183,239,200));
    setDifBox.push_back(sub1);

    // =======  SETTING DIFF ======

    // =======   WIN SCREEN  ======
    std::vector<sf::Text> winText;

    sf::RectangleShape winBG(conf::window_size_f);
    winBG.setTexture(&Win_BG);

    sf::Text winText1(conf::scoreFont);
    winText1.setCharacterSize(65);
    winText1.setString("0");
    winText1.setFillColor(sf::Color::Green);
    winText1.setOrigin({ winText1.getGlobalBounds().getCenter().x, winText1.getGlobalBounds().getCenter().y });
    winText1.setPosition({ conf::window_size_f.x / 2 - 300.f, conf::window_size_f.y / 4 * 2 });
    winText.push_back(winText1);

    // text
    sf::Text winText2(conf::playerFont);
    winText2.setCharacterSize(75);
    winText2.setFillColor(sf::Color::Green);
    winText2.setString("WRITE YOUR NAME PLEASE");
    winText2.setOrigin({ winText2.getGlobalBounds().getCenter().x, winText2.getGlobalBounds().getCenter().y });
    winText2.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 4 * 3 - 100.f });
    winText.push_back(winText2);

    // input text
    sf::Text winInput(conf::playerFont);
    winInput.setCharacterSize(75);
    winInput.setFillColor(sf::Color::Cyan);
    winInput.setOrigin({ winInput.getGlobalBounds().getCenter().x, winInput.getGlobalBounds().getCenter().y });
    winInput.setPosition({ conf::window_size_f.x / 2, conf::window_size_f.y / 4 * 3 + 100.f });
    winInput.setString("");
    sf::String input;

    // =======   WIN SCREEN  ======
    // <-----------  Making Programs ------------->

    states.push(GameStates::Menu);

    std::cout << "Displaying Apps...\n";
    std::cout << conf::window_size_f.x << ", " << conf::window_size_f.y << "\n";

    // ----------->   Begin Program  <-------------
    while (window.isOpen())
    {
        // --> Reset Clock
        sf::Time dt = clock.restart();  // time since last frame
        float deltaTime = dt.asSeconds();

        // --> Check Closed Condition
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (states.top() == GameStates::winScreen)
            {
                if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
                {
                    if (textEntered->unicode > 32 && textEntered->unicode < 128)
                    input += static_cast<char>(textEntered->unicode);
                    else if (textEntered->unicode == 8 && !input.isEmpty())
                    input.erase(input.getSize() - 1 , 1);
                }
                
                winInput.setString(input);
                winInput.setOrigin({winInput.getLocalBounds().getCenter().x , winInput.getLocalBounds().getCenter().y});
                winInput.setPosition({conf::window_size_f.x / 2, conf::window_size_f.y / 4 * 3 + 100.f});
            }
        }
        goPause();

        // Check popcount
        while (popCount > 0 && !states.empty())
        {
            std::cout << "Popping State: " << popCount << "\n";
            popCount--;
            states.pop();
            stateJustChanged = true;
        }
        
        // --> Clear window
        window.clear(sf::Color::White);
        
        // --> Drawing Window with met condition
        handleInputBuffer();
        if (states.top() == GameStates::Menu)
            menu(window, sprite, menuBox, menu_pos, 5);
        else if (states.top() == GameStates::SetDif)
            setDif(window, setDifText, setDifBox, player, platforms, rng, gameClock);
        else if (states.top() == GameStates::Score)
            score(window);
        else if (states.top() == GameStates::Playing)
        {
            game(window, view, player, platforms, deltaTime);
            
            window.setView(window.getDefaultView());

            float elapsed = getElapsedTime();
            int seconds = static_cast<int>(elapsed);
            stopwatchText.setString("Time : " + std::to_string(seconds));
            winText[0].setString(std::to_string(seconds)); // Assuming 10 points per second
            
            // text.setString("X: " + std::to_string((int)view.getCenter().x) + ", Y: " + std::to_string((int)view.getCenter().y) + " --> Camera Position\nX: " + std::to_string((int)player.getPosition().x) + ", Y: " + std::to_string(player.getPosition().y)+ " --> Player Position\nGround: " + std::to_string(posGround) + "\nVelocityY : " + std::to_string(velocityY) + "\nisOnGround : " + std::to_string(isOnGround));
            // window.draw(text);
            
            window.draw(stopwatchText);
            window.setView(view);
        }
        else if (states.top() == GameStates::Pause)
            Pause(window, pause_text, pause_box);
        else if (states.top() == GameStates::winScreen)
        {
            winScreen(window, winText, winBG, input);
            window.setView(window.getDefaultView());
            window.draw(winInput);
        }
        window.display();
    }
    std::cout << "Program Finished...\n";
}

void menu(sf::RenderWindow& window, sf::Sprite& sprite, sf::RectangleShape* box, int& pos, std::size_t count)
{
    window.setView(window.getDefaultView());

    sf::Vector2f opt({box[4].getPosition().x, box[4].getPosition().y});
    float step  = conf::window_size_f.y / 5; // jarak antar box
    // Proses input dari buffer
    while (!inputBuffer.empty()) {
        InputEvent input = inputBuffer.front();
        inputBuffer.pop();
        
        if (input == InputEvent::Up && pos > 1) {
            opt.y -= step;
            pos--;
        }
        else if (input == InputEvent::Down && pos < 3) {
            opt.y += step;
            pos++;
        }
        else if (input == InputEvent::Enter) {
            if (pos == 1)
            {
                states.push(GameStates::SetDif);
                clearInputBuffer();
                stateJustChanged = true;
            }
            else if (pos == 2)
            {
                states.push(GameStates::Score);
                clearInputBuffer();
                stateJustChanged = true;
            }
            else if (pos == 3)
            {
                clearInputBuffer();
                stateJustChanged = true;
                window.close();
            }
        }
    }
    clearInputBuffer();
    
    box[4].setPosition(opt);

    window.draw(sprite);
    for (std::size_t i = 0; i < count; i++) 
        window.draw(box[i]);
}

bool checkHorizontal(sf::Sprite player, std::vector<Platform> platforms, float count)
{
    player.move({ count, 0.f });

    for (const auto& plat : platforms)
    {
        sf::FloatRect platBounds = plat.shape.getGlobalBounds();

        if (player.getGlobalBounds().findIntersection(platBounds))
        {
            return true;
        }
    }
    return false;
}

void checkVertical(sf::Sprite& player, std::vector<Platform>& platforms)
{
    for (const auto& plat : platforms)
    {
        sf::FloatRect platBounds = plat.shape.getGlobalBounds();
        player.move({ 0.f, -0.5f });
        if (player.getGlobalBounds().findIntersection(platBounds) && velocityY < 0 && player.getGlobalBounds().getCenter().y > platBounds.getCenter().y)
        {
            velocityY = 10.f;
            isOnGround = false;
            player.move({ 0.f, 1.f });
            return;
        }

        player.move({ 0.f, 1.f });
        if (player.getGlobalBounds().findIntersection(platBounds) && velocityY >= 0 && player.getGlobalBounds().getCenter().y < platBounds.getCenter().y)
        {
            posGround = platBounds.position.y; // posisi atas platform
            velocityY = 0.f;
            isOnGround = true;
            player.move({ 0.f, -1.f });
            return;
        }
        player.move({ 0.f, -0.5f });
    }
    isOnGround = false; // Reset dulu
}

int CheckPlatform(sf::Sprite player, std::vector<Platform> platforms)
{
    player.setScale({ player.getScale().x + 1.f, player.getScale().y + 1.f});
    //player.setOrigin({ player.getGlobalBounds().getCenter().x, player.getGlobalBounds().getCenter().y });
    for (auto plat : platforms)
    {
        sf::FloatRect platBounds = plat.shape.getGlobalBounds();
        if (player.getGlobalBounds().findIntersection(platBounds))
        {
            if (plat.spikes)
                return 1;
            else if (plat.finishLine)
                return 2;
            else return 0;
        }
    }
    if (player.getPosition().y >= 1180) return 1;
    return 0;
}

void game(sf::RenderWindow& window, sf::View& cam, sf::Sprite& player, std::vector<Platform>& platforms, float deltaTime)
{
    // ----------->    Initialize    <-------------
    sf::Vector2f setcam = cam.getCenter();
    sf::Vector2f setP = player.getPosition();

    // <-----------    Initialize    ------------->

    // ----------->  Check Platform  <-------------
    int Condition = CheckPlatform(player, platforms);
    switch (Condition)
    {
    case 1:
        setP.x = conf::resetPos.x;
        setP.y = conf::resetPos.y;
        //resettime
        break;
    case 2:
        states.push(GameStates::winScreen);
        scores += gameClock.getElapsedTime().asSeconds();
        stateJustChanged = true;
        break;
    default:
        break;
    }
    // <-----------  Check Platform  ------------->
    
    // ----------->   Moving Player  <-------------
    // Left and Right
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && setP.x + 2.f <= conf::MaxBorder.x && !checkHorizontal(player, platforms, 3.f))
        setP.x += 3.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && setP.x - 2.f >= conf::MinBorder.x && !checkHorizontal(player, platforms, -3.f))
        setP.x -= 3.f;

    // Input SpaceBar
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) && isOnGround) {
        velocityY = conf::jumpVelocity;  // apply jump impulse
        isOnGround = false;
    }
    // <-----------   Moving Player  ------------->

    // ----------->   Jumping Logic  <-------------
    // Applying Gravity
    if (!isOnGround) 
    {
        // Cek apakah spasi masih ditekan saat di udara
        bool spaceHeld = (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W));

        // Terapkan gravitasi ringan jika spasi ditekan, gravitasi normal jika dilepas
        float gravityToApply = spaceHeld ? conf::gravity * 0.9f : conf::gravity * 1.8f;

        velocityY += gravityToApply * deltaTime;
        setP.y += velocityY * deltaTime;
        player.setPosition(setP);
    }
    else player.setPosition({ setP.x, posGround - 0.2f}); // origin bottom center, jadi ini 

    // Handle Ground Position
    checkVertical(player, platforms);

    // <-----------   Jumping Logic  ------------->

    // ----------->   Moving Camera  <-------------
    sf::Vector2f camCenter = cam.getCenter();

    float targetCamX = setP.x;
    float targetCamY = setP.y;

    // Lerp untuk sumbu X dan Y
    camCenter.x += (targetCamX - camCenter.x) * conf::CameraSmoothness * deltaTime;
    camCenter.y += (targetCamY - camCenter.y) * conf::CameraSmoothness * deltaTime;

    // Clamp kamera secara horizontal
    float minCamX = conf::MinBorder.x + cam.getSize().x / 2;
    float maxCamX = conf::MaxBorder.x - cam.getSize().x / 2;
    camCenter.x = std::clamp(camCenter.x, minCamX, maxCamX);

    // Clamp kamera secara vertikal (sesuaikan batas bawah dan atas dunia Anda)
    float minCamY = conf::MinBorder.y + cam.getSize().y / 2;
    float maxCamY = conf::MaxBorder.y - cam.getSize().y / 2;
    camCenter.y = std::clamp(camCenter.y, minCamY, maxCamY);

    // Update posisi kamera
    cam.setCenter(camCenter);
    // <-----------   Moving Camera  ------------->

    // ----------->    Set Camera    <-------------
    for (const auto& platform : platforms)
        window.draw(platform.shape);
    window.draw(player);
    window.setView(cam);
}

void ShuffleRNG(std::mt19937& rng)
{
    auto timeSeed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::random_device rd;
    std::seed_seq seedSeq{ rd(), timeSeed };
    rng.seed(seedSeq);
}

void generatePlatforms
(
    std::vector<Platform>& platforms, 
    float startX, float startY, 
    int count, 
    std::mt19937& rng,
    int spikeStreak, 
    int groundStreak
)
{
    if (count <= 0) return;

    // Cek apakah finishLine sudah ada di platforms
    for (const auto& p : platforms) {
        if (p.finishLine) {
            // Jika sudah ada finish line, hentikan generate
            return;
        }
    }

    std::uniform_real_distribution<float> distX(150.f, 300.f); // jarak horizontal antar platform
    std::uniform_real_distribution<float> distSpikesX(150.f, 200.f); // jarak horizontal antar spike
    std::uniform_real_distribution<float> distY(-100.f, 100.f); // variasi vertikal
    //std::uniform_real_distribution<float> heightChance(20.f, 80.f); // tinggi platform
    std::uniform_int_distribution<int> heightChance(0, 3); // is Tall or No
    std::uniform_int_distribution<int> NON(0, 2); // Next Spikes or No
    std::uniform_int_distribution<int> NGN(0, 2); // Next Ground or No

    float nextX;
    float nextY;

    if (platforms.empty()) {
        // Platform pertama di X=0, Y startY (misal ground)
        nextX = 0.f;
        nextY = startY;
        spikeStreak = 0;
        groundStreak = 1;
    }
    else {
        float prevX = platforms.back().shape.getPosition().x;
        float prevY = platforms.back().shape.getPosition().y;
        bool prevSpike = platforms.back().spikes;

        nextX = prevX + ((mergePlatform) ? 150.f : distX(rng));
        if (spikeStreak == 1)
            nextX = prevX + ((mergePlatform) ? 150.f : distSpikesX(rng));

        // Logika Y untuk spike dan ground
        if (prevSpike) {
            // Jika platform sebelumnya spike
            if (spikeStreak == 1) {
                // Spike kedua berturut-turut, harus di bawah spike pertama
                nextY = std::min(prevY + 50.f, 1100.f);
                spikeStreak = (NON(rng) == 0) ? 0 : 2 ;
                groundStreak = 0;
            }
            else {
                // Setelah 2 spike, ground harus di bawah spike kedua
                nextY = std::min(prevY + 80.f, 1150.f);
                spikeStreak = 0;
                groundStreak = 1;
            }
        }
        else {
            // Platform sebelumnya ground
            if (groundStreak >= 4) {
                // Maksimal 4 ground berturut-turut, paksa spike
                nextY = std::clamp(prevY + distY(rng), 300.f, 1150.f);
                spikeStreak = 1;
                groundStreak = 0;
            }
            else {
                // Ground biasa, variasi Y
                nextY = std::clamp(prevY + distY(rng), 300.f, 1150.f);
                spikeStreak = 0;
                groundStreak = (NGN(rng) == 0) ? 5 : groundStreak + 1;
            }
        }
    }

    float height = (heightChance(rng) == 0) ? 20.f : 500.f;

    Platform plat;
    plat.shape.setSize({ 150.f, height });
    plat.shape.setPosition({ nextX, nextY });

    // Atur warna dan spikes
    if (spikeStreak > 0) {
        plat.spikes = true;
        plat.shape.setFillColor(sf::Color::Red);
    }
    else {
        // Platform dasar jika Y mendekati 1000
        if (nextY > 1100.f) {
            plat.shape.setFillColor(sf::Color(100, 50, 0)); // coklat tanah
        }
        else {
            plat.shape.setFillColor(sf::Color::Green);
        }
    }

    // Finish line pada platform terakhir atau probabilitas kecil
    if (count == 1 || nextX >= conf::MaxBorder.x - 280.f) {
        count = 0;
        spikeStreak = 0;
        plat.spikes = false;
        plat.finishLine = true;
        plat.shape.setFillColor(sf::Color::Blue);
        platforms.push_back(plat);
        return; // jangan generate lagi setelah finish line
    }

    platforms.push_back(plat);

    // Rekursif ke platform berikutnya dengan update streak
    generatePlatforms(platforms, nextX, nextY, count - 1, rng, spikeStreak, groundStreak);
}

void goPause()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        if (states.top() == GameStates::Playing)
        {
            pauseClock();
            states.push(GameStates::Pause);
            stateJustChanged = true;
        }
    }
}

void Pause(sf::RenderWindow& window, std::vector<sf::Text> texts, sf::RectangleShape& box)
{
    window.setView(window.getDefaultView());

    while (!inputBuffer.empty()) 
    {
        InputEvent input = inputBuffer.front();
        inputBuffer.pop();

        float posY = box.getPosition().y;
        float step = conf::window_size_f.y / 5;
        float offset = 50.f;

        if (input == InputEvent::Up) {
            if (posY > step * 2 + offset) {
                box.move({0.f, -step});
            }
        }
        else if (input == InputEvent::Down) {
            if (posY < step * 4 + offset) {
                box.move({0.f, step});
            }
        }
        else if (input == InputEvent::Enter) {
            if (posY == step * 2 + offset) {
                popCount = 1;
                resumeClock();
                clearInputBuffer();
            }
            else if (posY == step * 3 + offset) {
                popCount = 3;
                clearInputBuffer();
            }
            else if (posY == step * 4 + offset) {
                window.close();
            }
        }
    }


    window.draw(box);
    for (const auto& text : texts)
        window.draw(text);
}

void handleInputBuffer() 
{
    static bool wLast = false, sLast = false, enterLast = false, aLast = false, dLast = false;
    bool wNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    bool sNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool aNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    bool dNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    bool enterNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);

    if (wNow && !wLast) inputBuffer.push(InputEvent::Up);
    if (sNow && !sLast) inputBuffer.push(InputEvent::Down);
    if (aNow && !aLast) inputBuffer.push(InputEvent::Left);
    if (dNow && !dLast) inputBuffer.push(InputEvent::Right);
    if (enterNow && !enterLast) inputBuffer.push(InputEvent::Enter);

    wLast = wNow;
    sLast = sNow;
    aLast = aNow;
    dLast = dNow;
    enterLast = enterNow;
}

void clearInputBuffer() 
{
    while (!inputBuffer.empty())
        inputBuffer.pop();
}

void appendScoresToJson(const std::string &filename, const std::string &muchplatformKey, const std::string &mergeKey, const std::map<std::string, int> &newScores)
{
    // Baca file JSON yang sudah ada
    std::ifstream inFile(filename);
    json j;
    if (inFile.is_open()) {
        try {
            inFile >> j;
        } catch (...) {
            // Jika file kosong atau rusak, inisialisasi json kosong
            j = json::object();
        }
        inFile.close();
    } else {
        // Jika file belum ada, buat objek kosong
        j = json::object();
    }

    // Pastikan struktur kategori ada
    if (!j.contains("Runners")) {
        j["Runners"] = json::object();
    }

    // Pastikan muchplatformKey ada
    if (!j["Runners"].contains(muchplatformKey)) {
        j["Runners"][muchplatformKey] = json::object();
    }

    // Pastikan mergeKey ada
    if (!j["Runners"][muchplatformKey].contains(mergeKey)) {
        j["Runners"][muchplatformKey][mergeKey] = json::object();
    }

    // Pastikan "scores" ada dan bertipe objek
    if (!j["Runners"][muchplatformKey][mergeKey].contains("scores") || 
        !j["Runners"][muchplatformKey][mergeKey]["scores"].is_object()) {
        j["Runners"][muchplatformKey][mergeKey]["scores"] = json::object();
    }

    // Tambah atau update skor baru ke dalam scores
    for (const auto& [player, score] : newScores) {
        j["Runners"][muchplatformKey][mergeKey]["scores"][player] = score;
    }

    // Tulis ulang ke file
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Gagal membuka file untuk menulis");
    }
    outFile << j.dump(4);
    outFile.close();
}

void setDif(sf::RenderWindow &window, std::vector<sf::Text> &texts, std::vector<sf::RectangleShape> &box, sf::Sprite& player, std::vector<Platform>& platforms, std::mt19937& rng, sf::Clock& clock)
{
    window.setView(window.getDefaultView());

    while (!inputBuffer.empty()) {
        InputEvent input = inputBuffer.front();
        inputBuffer.pop();

        if (input == InputEvent::Up && box[1].getPosition().x == texts[2].getPosition().x )
        {
            box[1].setPosition((mergePlatform == true) ? texts[0].getPosition() : texts[1].getPosition());
        }
        else if (input == InputEvent::Down && box[1].getPosition().x != texts[2].getPosition().x)
        {
            box[1].setPosition(texts[2].getPosition());
        }
        else if (input == InputEvent::Left)
        {
            if (box[1].getPosition().y == texts[2].getPosition().y)
            {
                muchPlatform = (muchPlatform == 30) ? 60 : 30;
                texts[2].setString(std::to_string(muchPlatform));
            }
            else if (box[1].getPosition().y != texts[2].getPosition().y && !mergePlatform)
            {
                mergePlatform = true;
                box[1].setPosition(texts[0].getPosition());
            }
        }
        else if (input == InputEvent::Right)
        {
            if (box[1].getPosition().y == texts[2].getPosition().y)
            {
                muchPlatform = (muchPlatform == 30) ? 60 : 30;
                texts[2].setString(std::to_string(muchPlatform));
            }
            else if (box[1].getPosition().y != texts[2].getPosition().y && mergePlatform)
            {
                mergePlatform = false;
                box[1].setPosition(texts[1].getPosition());
            }
        }
        else if (input == InputEvent::Enter) 
        {
            states.push(GameStates::Playing);
            stateJustChanged = true;
            player.setPosition(conf::resetPos);
            ShuffleRNG(rng);
            platforms.clear();
            generatePlatforms(platforms, 0.f, conf::Ground, muchPlatform, rng);
            gameClock.restart(); // Reset clock for new game
            pauseTime = 0.f; // Reset pause time
            isPaused = false; // Reset pause state
            clearInputBuffer();
            return; // Exit after starting the game
        }
    }

    for(auto b : box)
        window.draw(b);
    for(auto text : texts)
        window.draw(text);
}

void score(sf::RenderWindow &window)
{
    window.setView(window.getDefaultView());
    while (!inputBuffer.empty()) {
        InputEvent input = inputBuffer.front();
        inputBuffer.pop();

        if (input == InputEvent::Enter) 
        {
            states.pop();
            stateJustChanged = true;
            clearInputBuffer();
            return; // Exit after starting the game
        }
    }
}

void winScreen(sf::RenderWindow &window, std::vector<sf::Text> &texts, sf::RectangleShape bg, sf::String inputText)
{
    window.setView(window.getDefaultView());
    // gambar BG
    window.draw(bg);

    // Gambar semua teks
    for (const auto& text : texts)
        window.draw(text);

    // Proses input dari buffer
    while (!inputBuffer.empty()) {
        InputEvent input = inputBuffer.front();
        inputBuffer.pop();

        if (input == InputEvent::Enter && !inputText.isEmpty())
        {
            popCount = 3;
            clearInputBuffer();
            stateJustChanged = true;
            std::map<std::string, int> scoresToAdd = {{inputText.toAnsiString(), (int)scores}};
            appendScoresToJson(conf::ScoresFile, (muchPlatform == 30) ? "muchplatform_30" : "muchplatform_60" , (mergePlatform == true) ? "merge_true" : "merge_false", scoresToAdd);
            break;
        }
    }
}

float getElapsedTime()
{
    if (isPaused) {
        // Jika pause, kembalikan waktu sebelum pause
        return pauseTime;
    } else {
        // Jika tidak pause, total waktu = waktu sebelum pause + waktu sejak resume
        return pauseTime + gameClock.getElapsedTime().asSeconds();
    }
}

void pauseClock()
{
    if (!isPaused) {
        pauseTime += gameClock.getElapsedTime().asSeconds();
        isPaused = true;
    }
}

void resumeClock()
{
    if (isPaused) {
        gameClock.restart();  // reset clock untuk hitung waktu baru saat resume
        isPaused = false;
    }
}