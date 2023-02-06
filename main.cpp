#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// get error events before crating window
// events are reported through callbacks
GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun cbfun);
// call back
// first arguments is the error code, second is description
typedef void(*GLFWerrorfun) (int, const char*);

// print the error
void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// for cpu base rendering
struct Buffer
{
    size_t width, height;
    uint32_t* data;
};

struct Sprite
{
    size_t width, height;
    uint8_t* data;
};

struct Alien
{
    size_t x, y; // position
    uint8_t type; // alien type
};

struct Player
{
    size_t x, y; // position
    size_t life; // health
};

// struct for all game related variables
struct Game
{
    size_t width, height; // size of the game
    size_t numAliens;
    Alien* aliens; // alies as dynamically allocated array
    Player player;
};

// struct for animations
struct SpriteAnimation
{
    bool loop;
    size_t numFrames;
    size_t frameDuration;
    size_t time;
    Sprite** frames;
};

// to help us define colors
uint32_t rgbToUint32 (uint8_t r, uint8_t g, uint8_t b)
{
    // set left 24 bits to r, g, b and right 8 bits to 225 for alpha
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

// iterate over all pixels and sets each pixel to the given color
void bufferClear(Buffer* buffer, uint32_t color)
{
    for (size_t i = 0; i < buffer->width * buffer->height; i++)
    {
        buffer->data[i] = color;
    }
}

// func to validate shader
void validateShader(GLuint shader, const char* file = 0)
{
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if (length > 0)
    {
        printf("Shader %d(%s) compile error: %s\n", shader, (file ? file : ""), buffer);
    }
}

// func to validate program
bool validateProgram(GLuint program)
{
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if (length > 0)
    {
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }
    return true;
}

void bufferDrawSprite(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color)
{
    for (size_t xi = 0; xi < sprite.width; ++xi)
    {
        for (size_t yi = 0; yi < sprite.height; ++yi)
        {
            if(sprite.data[yi * sprite.width + xi] &&
               (sprite.height - 1 + y - yi) < buffer->height &&
               (x + xi) < buffer->width)
            {
                buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = color;
            }
        }
    }
}

// test call back
bool gameRunning = false;

// key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS) gameRunning = false;
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[])
{
    // set buffer sizes
    const size_t bufferWidth = 224;
    const size_t bufferHeight = 256;

    glfwSetErrorCallback(errorCallback);

    // initialize glfw
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(2 * bufferWidth, 2 * bufferHeight, "スペースインベーダー", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing GLEW.\n");
        glfwTerminate();
        return -1;
    }
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(1.0, 0.0, 0.0, 1.0);

    // create graphics buffer
    Buffer buffer;
    buffer.width  = bufferWidth;
    buffer.height = bufferHeight;
    buffer.data   = new uint32_t[buffer.width * buffer.height];
    bufferClear(&buffer, 0); //set color

    // create texture for presenting buffer to OpenGL
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // Create vao for generating fullscreen triangle
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);


    // shader for drawing verticies
    static const char* vertexShader =
        "\n"
        "#version 330\n"
        "\n"
        "noperspective out vec2 TexCoord;\n"
        "\n"
        "void main(void){\n"
        "\n"
        "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
        "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
        "    \n"
        "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
        "}\n";

    // shader for displaying buffer / colors
    static const char* fragmentShader =
        "\n"
        "#version 330\n"
        "\n"
        "uniform sampler2D buffer;\n"
        "noperspective in vec2 TexCoord;\n"
        "\n"
        "out vec3 outColor;\n"
        "\n"
        "void main(void){\n"
        "    outColor = texture(buffer, TexCoord).rgb;\n"
        "}\n";

    GLuint shader_id = glCreateProgram();

    {
        //Create vertex shader
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(shader_vp, 1, &vertexShader, 0);
        glCompileShader(shader_vp);
        validateShader(shader_vp, vertexShader);
        glAttachShader(shader_id, shader_vp);

        glDeleteShader(shader_vp);
    }

    {
        //Create fragment shader
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(shader_fp, 1, &fragmentShader, 0);
        glCompileShader(shader_fp);
        validateShader(shader_fp, fragmentShader);
        glAttachShader(shader_id, shader_fp);

        glDeleteShader(shader_fp);
    }

    // link the shader
    glLinkProgram(shader_id);

    if(!validateProgram(shader_id)){
        fprintf(stderr, "Error while validating shader.\n");
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }

    // enable the shader program
    glUseProgram(shader_id);

    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);


    //OpenGL setup
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(fullscreen_triangle_vao);

    // create and draw the sprite
    Sprite alienSprite0;
    alienSprite0.width = 11;
    alienSprite0.height = 8;
    alienSprite0.data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
        0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
        0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
        0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };

    // create and draw sprite frame
    Sprite alienSprite1;
    alienSprite1.width = 11;
    alienSprite1.height = 8;
    alienSprite1.data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };

    // create a two frame animation using the two alien sprites
    SpriteAnimation* alienAnimation = new SpriteAnimation;

    alienAnimation->loop = true;
    alienAnimation->numFrames = 2;
    alienAnimation->frameDuration = 10;
    alienAnimation->time = 0;

    alienAnimation->frames = new Sprite* [2];
    alienAnimation->frames[0] = &alienSprite0;
    alienAnimation->frames[1] = &alienSprite1;

    // turn on vsync
    glfwSwapInterval(1);

    // create and draw the player
    Sprite playerSprite;
    playerSprite.width = 11;
    playerSprite.height = 7;
    playerSprite.data = new uint8_t[77]
    {
        0,0,0,0,0,1,0,0,0,0,0, // .....@.....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    };

    // create and initialize game struct
    Game game;
    game.width = bufferWidth;
    game.height = bufferHeight;
    game.numAliens = 55; // set number of aliens
    game.aliens = new Alien[game.numAliens];

    // set position near the bottom of the screen
    game.player.x = 112 - 5;
    game.player.y = 32;

    game.player.life = 3; // 3 lives

    // initialize alien positions
    for (size_t yi = 0; yi < 5; ++yi)
    {
        for (size_t xi = 0; xi < 11; ++xi)
        {
            game.aliens[yi * 11 + xi].x = 16 * xi + 20;
            game.aliens[yi * 11 + xi].y = 17 * yi + 128;
        }
    }

    uint32_t clearColor = rgbToUint32(0, 128, 0);

    // player movement variable
    int playerMoveDir = 1;

    gameRunning = true;

    // main loop
    while (!glfwWindowShouldClose(window) && gameRunning)
    {
        bufferClear(&buffer, clearColor);

        // draw the player and all aliens
        for (size_t ai = 0; ai < game.numAliens; ++ai)
        {
            const Alien& alien = game.aliens[ai];

            size_t currentFrame = alienAnimation->time / alienAnimation->frameDuration;
            const Sprite& sprite = *alienAnimation->frames[currentFrame];
            bufferDrawSprite(&buffer, sprite, alien.x, alien.y, rgbToUint32(128, 0, 0));
        }

        bufferDrawSprite(&buffer, playerSprite, game.player.x, game.player.y, rgbToUint32(128, 0, 0));

        // at the end of each frame we update all animations by advancingmtime.
        // if animation reached its end, delete it or set time back to 0 if its looping
        ++alienAnimation->time;
        if (alienAnimation->time == alienAnimation->numFrames * alienAnimation->frameDuration)
        {
            if (alienAnimation->loop) alienAnimation->time = 0;
            else
            {
                delete alienAnimation;
                alienAnimation = nullptr;
            }
        }

        // updating player movement at the end of each frame based on it
        if (game.player.x + playerSprite.width + playerMoveDir >= game.width - 1)
        {
            game.player.x = game.width - playerSprite.width - playerMoveDir - 1;
            playerMoveDir *= -1;
        }
        else if ((int)game.player.x + playerMoveDir <= 0)
        {
            game.player.x = 0;
            playerMoveDir *= -1;
        }
        else game.player.x += playerMoveDir;

        // draw the a red sprite at position 112, 128
        // bufferDrawSprite(&buffer, alienSprite, 112, 128, rgbToUint32(128, 0, 0));

        // draw the sprite each frame of the game loop
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            buffer.width, buffer.height,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
            buffer.data
        );

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    glDeleteVertexArrays(1, &fullscreen_triangle_vao);

    delete[] alienAnimation->frames;
    delete[] alienSprite1.data;
    delete[] alienSprite0.data;
    delete[] buffer.data;
    delete[] game.aliens;

    return 0;
}
