#ifndef    APP_DECORATOR_HH
# define   APP_DECORATOR_HH

# include <memory>
# include <sdl_engine/EngineDecorator.hh>
# include <sdl_engine/Palette.hh>

namespace sdl {
  namespace app {

    class AppDecorator: public core::engine::EngineDecorator {
      public:

        AppDecorator(core::engine::EngineShPtr engine,
                     const utils::Uuid& canvas,
                     const core::engine::Palette& palette,
                     const utils::Uuid& window);

        virtual ~AppDecorator();

        void
        setDrawingCanvas(const utils::Uuid& canvas);

        void
        clearWindow(const utils::Uuid& uuid) override;

        void
        renderWindow(const utils::Uuid& uuid) override;

        utils::Uuid
        createTexture(const utils::Uuid& win,
                      const utils::Sizef& size,
                      const core::engine::Palette::ColorRole& role) override;

        utils::Uuid
        createTexture(const utils::Sizef& size,
                      const core::engine::Palette::ColorRole& role) override;

        utils::Uuid
        createTextureFromFile(const utils::Uuid& win,
                              core::engine::ImageShPtr img,
                              const core::engine::Palette::ColorRole& role) override;

        utils::Uuid
        createTextureFromFile(core::engine::ImageShPtr img,
                              const core::engine::Palette::ColorRole& role) override;

        utils::Uuid
        createTextureFromText(const utils::Uuid& win,
                              const std::string& text,
                              const utils::Uuid& font,
                              const core::engine::Palette::ColorRole& role) override;

        utils::Uuid
        createTextureFromText(const std::string& text,
                              const utils::Uuid& font,
                              const core::engine::Palette::ColorRole& role) override;

        void
        drawTexture(const utils::Uuid& tex,
                    const utils::Boxf* from = nullptr,
                    const utils::Uuid* on = nullptr,
                    const utils::Boxf* where = nullptr) override;

      private:

        utils::Uuid m_canvas;
        core::engine::Palette m_palette;
        utils::Uuid m_window;
    };

    using AppDecoratorShPtr = std::shared_ptr<AppDecorator>;
  }
}

# include "AppDecorator.hxx"

#endif    /* APP_DECORATOR_HH */
