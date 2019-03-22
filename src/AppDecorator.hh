#ifndef    APP_DECORATOR_HH
# define   APP_DECORATOR_HH

# include <memory>
# include <sdl_engine/EngineDecorator.hh>

namespace sdl {
  namespace app {

    class AppDecorator: public core::engine::EngineDecorator {
      public:

        AppDecorator(core::engine::EngineShPtr engine,
                     const utils::Uuid& canvas,
                     const utils::Uuid& window);

        virtual ~AppDecorator();

        utils::Uuid
        createTexture(const utils::Uuid& win,
                      const utils::Sizei& size) override;

        utils::Uuid
        createTexture(const utils::Sizei& size) override;

        utils::Uuid
        createTextureFromFile(const utils::Uuid& win,
                              const std::string& file) override;

        utils::Uuid
        createTextureFromFile(const std::string& file) override;
        
        utils::Uuid
        createTextureFromText(const utils::Uuid& win,
                              const std::string& text,
                              core::engine::ColoredFontShPtr font) override;

        utils::Uuid
        createTextureFromText(const std::string& text,
                              core::engine::ColoredFontShPtr font) override;

        void
        drawTexture(const utils::Uuid& tex,
                    const utils::Uuid* on = nullptr,
                    utils::Boxf* where = nullptr) override;

      private:

        utils::Uuid m_canvas;
        utils::Uuid m_window;
    };

    using AppDecoratorShPtr = std::shared_ptr<AppDecorator>;
  }
}

# include "AppDecorator.hxx"

#endif    /* APP_DECORATOR_HH */
