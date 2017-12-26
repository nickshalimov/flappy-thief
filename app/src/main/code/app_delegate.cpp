#include "app_clock.h"
#include "asset_loader.h"
#include "bundle.h"
#include "game.h"
#include "renderer.h"

#include <android_native_app_glue.h>
#include <android/window.h>

#include <algorithm>
#include <fstream>
#include <sstream>

const int64_t DELTA_TIME = 1000 / 60;

class app_delegate final
{
public:
    explicit app_delegate(android_app* native_app);
    ~app_delegate();

    void run();

private:
    android_app* app_;
    asset_loader loader_;
    std::string score_path_;
    app_clock clock_;
    bundle bundle_;
    std::unique_ptr<game> game_;
    std::unique_ptr<renderer> renderer_;

    void handle_command(int32_t command);
    int32_t handle_input(AInputEvent* event);
};

void save_value(const std::string& path, uint32_t value)
{
    std::ofstream stream { path };
    stream << value;
    stream.close();
}

uint32_t load_value(const std::string& path)
{
    std::ifstream stream { path };
    if (!stream.is_open()) { return 0; }

    uint32_t score;
    stream >> score;
    return score;
}

app_delegate::app_delegate(android_app* native_app)
        : app_(native_app)
        , loader_(app_->activity->assetManager)
        , score_path_(std::string(app_->activity->internalDataPath) + "/points")
{
    ANativeActivity_setWindowFlags(
        app_->activity,
        AWINDOW_FLAG_FULLSCREEN,
        AWINDOW_FLAG_FULLSCREEN
    );

    app_->userData = this;
    app_->onAppCmd = [](android_app* app, int32_t command)
    {
        static_cast<app_delegate*>(app->userData)->handle_command(command);
    };
    app_->onInputEvent = [](android_app* app, AInputEvent* event)
    {
        return static_cast<app_delegate*>(app->userData)->handle_input(event);
    };
}

app_delegate::~app_delegate()
{
    app_->onAppCmd = nullptr;
    app_->onInputEvent = nullptr;
    app_->userData = nullptr;
}

void app_delegate::run()
{
    std::stringstream { loader_.load_string("bundle.txt") } >> bundle_;

    game_.reset(new game(load_value(score_path_), bundle_));

    int64_t current_time = clock_.now();
    int64_t game_time = 0;
    int64_t accumulator = 0;

    int events;
    android_poll_source* source;

    int64_t time;

    while (true)
    {
        time = clock_.now();

        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0)
        {
            if (source != nullptr) { source->process(app_, source); }
            if (app_->destroyRequested != 0) { return; }
        }

        if (clock_.is_paused()) { continue; }

        int64_t frame_time = std::min<int64_t>(time - current_time, 250);
        current_time = time;
        accumulator += frame_time;

        while (accumulator >= DELTA_TIME)
        {
            game_->integrate(DELTA_TIME);
            game_time += DELTA_TIME;
            accumulator -= DELTA_TIME;
        }

        if (renderer_ != nullptr)
        {
            renderer_->begin_frame(accumulator / float(DELTA_TIME), frame_time);
            game_->draw(renderer_.get());
            renderer_->end_frame();
        }
    }
}

void app_delegate::handle_command(int32_t command)
{
    switch (command)
    {
        case APP_CMD_INIT_WINDOW:
            renderer_.reset(new renderer(app_->window));
            renderer_->load_assets(bundle_, loader_);
            break;

        case APP_CMD_TERM_WINDOW:
            renderer_.reset();
            break;

        case APP_CMD_LOST_FOCUS:
            clock_.set_paused(true);
            save_value(score_path_, game_->best_score());
            break;

        case APP_CMD_GAINED_FOCUS:
            clock_.set_paused(false);
            break;

        default: break;
    }
}

int32_t app_delegate::handle_input(AInputEvent* event)
{
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        const auto action = AMotionEvent_getAction(event);
        if (action == AMOTION_EVENT_ACTION_DOWN)
        {
            game_->handle_tap_down();
        }
    }

    return 0;
}

void android_main(struct android_app* app)
{
    app_dummy();
    app_delegate(app).run();
}
