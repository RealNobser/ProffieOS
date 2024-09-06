#ifndef MODE_COLOR_MENUES_H
#define MODE_COLOR_MENUES_H

#include "mode.h"
#include "../common/color.h"
#include "style_argument_helpers.h"
#include "menu_list.h"

namespace mode {

// Note, the currently edited color is stored in ShowColorStyle.

template<class SPEC>
struct ColorHueMode : public SPEC::SmoothWraparoundMode {
public:
  void mode_activate(bool onreturn) override {
    getSL<SPEC>()->SayRotate();
    hsl_color = ShowColorStyle::getColor().toHSL();
    PVLOG_DEBUG << "H = " << hsl_color.H << "\n";
    SPEC::SmoothWraparoundMode::mode_activate(onreturn);
  }

  void select() override {
    getSL<SPEC>()->SaySelect();
    SPEC::SmoothWraparoundMode::select();
  }
  
  void exit() override {
    getSL<SPEC>()->SayCancel();
    SPEC::SmoothWraparoundMode::exit();
  }
  
  int get() override {
    return this->hsl_color.H * 32767.0f;
  }

  void set(int x) override {
    // Say number??
    this->hsl_color.H = x / 32767.0f;
    ShowColorStyle::SetColor(Color16(hsl_color));
  }

  HSL hsl_color;
};

#ifndef COLOR_MENU_GAMMA
#define COLOR_MENU_GAMMA 2.2
#endif

template<class SPEC>
struct ColorBrightnessMode : public SPEC::SmoothMode {
public:
  virtual float revolutions() { return 2.0f; }
  void mode_activate(bool onreturn) override {
    hsl_color = ShowColorStyle::getColor().toHSL();
    SPEC::SmoothMode::mode_activate(onreturn);
    getSL<SPEC>()->SayRotate();
    PVLOG_DEBUG << "init L = " << this->hsl_color.L << "\n";
    PVLOG_DEBUG << "init B = " << ShowColorStyle::getColor().b << "\n";
  }

  void select() override {
    getSL<SPEC>()->SaySelect();
    SPEC::SmoothMode::select();
  }
  
  void exit() override {
    getSL<SPEC>()->SayCancel();
    SPEC::SmoothMode::exit();
  }
  

  int get() override {
    float ret = this->hsl_color.L;
    if (ret > 0.5) {
      ret = powf((this->hsl_color.L - 0.5) * 2.0, 1.0/COLOR_MENU_GAMMA) / 2.0 + 0.5;
    } else {
      ret = powf(this->hsl_color.L * 2.0, 1.0/COLOR_MENU_GAMMA) / 2.0;
    }
    PVLOG_DEBUG << "L = " << this->hsl_color.L << "\n";
    return ret * 32767.0;
  }

  void set(int x) override {
    HSL hsl = this->hsl_color;
    hsl.L = x / 32767.0;
    if (hsl.L > 0.5) {
      hsl.L = powf( (hsl.L - 0.5) * 2.0, COLOR_MENU_GAMMA) / 2.0 + 0.5;
      hsl.S = 1.0;
    } else {
      hsl.L = powf( hsl.L * 2.0, COLOR_MENU_GAMMA) / 2.0;
      hsl.S = hsl.L * 2.0f;
    }
    ShowColorStyle::SetColor(Color16(hsl));
  }
  HSL hsl_color;
};


template<class SPEC>
struct ColorGammaMode : public SPEC::SmoothMode {
  virtual uint16_t* value() = 0;
  virtual float revolutions() { return 1.0f; }
  void mode_activate(bool onreturn) override {
    SPEC::SmoothMode::mode_activate(onreturn);
    getSL<SPEC>()->SayRotate();
  }
  int get() override {
    return powf(*value() / 65535.0, 1.0/COLOR_MENU_GAMMA) * 32767;
  }

  void set(int x) override {
    *value()= powf(x / 32767.0, COLOR_MENU_GAMMA) * 65535.0;
  }
};
  
template<class SPEC>
struct ColorRedMode : public SPEC::GammaMode {
  uint16_t* value() override { return &ShowColorStyle::getColor().r; }
};

template<class SPEC>
struct ColorGreenMode : public SPEC::GammaMode {
  uint16_t* value() override { return &ShowColorStyle::getColor().g; }
};

template<class SPEC>
struct ColorBlueMode : public SPEC::GammaMode {
  uint16_t* value() override { return &ShowColorStyle::getColor().b; }
};

Color16 menu_selected_color;

template<class SPEC>
struct SelectColorEntry : public MenuEntry {
  void say(int entry) override {
    getSL<SPEC>()->SayCopyColor();
  }
  void select(int entry) override {
    getSL<SPEC>()->SaySelect();
    menu_selected_color = ShowColorStyle::getColor();
  }
};
  
template<class SPEC>
struct UseSelectedColorEntry : public MenuEntry {
  void say(int entry) override {
    getSL<SPEC>()->SayPasteColor();
  }
  void select(int entry) override {
    getSL<SPEC>()->SaySelect();
    ShowColorStyle::SetColor(menu_selected_color);
  }
};

template<class SPEC>
struct ResetColorToDefaultEntry : public MenuEntry {
  void say(int entry) override {
    getSL<SPEC>()->SayResetToDefaultColor();
  }
  void select(int entry) override {
    getSL<SPEC>()->SaySelect();
    LSPtr<char> builtin = style_parser.ResetArguments(GetStyle(menu_current_blade));
    char argspace[32];
    style_parser.GetArgument(builtin.get(), menu_current_arg, argspace);
    char* tmp;
    int r = strtol(argspace, &tmp, 0);
    int g = strtol(tmp+1, &tmp, 0);
    int b = strtol(tmp+1, NULL, 0);
    ShowColorStyle::SetColor(Color16(r,g,b));
  }
};

template<class SPEC, class MENU>
struct SaveColorMenuEntry : public MenuEntry {
  void say(int entry) override {
    getSL<SPEC>()->SaySave();
  }
  void select(int entry) override {
    getSL<SPEC>()->SaySelect();
    getPtr<MENU>()->save();
    popMode();
  }
};

template<class SPEC, class MENU>
using ColorSelectList = MenuEntryMenu<SPEC,
  SubMenuEntry<typename SPEC::ColorHueMode, typename SPEC::SoundLibrary::tAdjustColorHue>,
  SubMenuEntry<typename SPEC::ColorBrightnessMode, typename SPEC::SoundLibrary::tEditBrightness>,
  SubMenuEntry<typename SPEC::ColorRedMode, typename SPEC::SoundLibrary::tAdjustRed>,
  SubMenuEntry<typename SPEC::ColorGreenMode, typename SPEC::SoundLibrary::tAdjustGreen>,
  SubMenuEntry<typename SPEC::ColorBlueMode, typename SPEC::SoundLibrary::tAdjustBlue>,
  typename SPEC::SelectColorEntry,
  typename SPEC::UseSelectedColorEntry,
  typename SPEC::ResetColorToDefaultEntry,
  SaveColorMenuEntry<SPEC, MENU>>;

template<class SPEC>
struct ColorSelectMode : public ColorSelectList<SPEC, ColorSelectMode<SPEC>> {

  virtual void load() {
    ShowColorStyle::SetColor(GetColorArg(menu_current_blade, menu_current_arg));
  }

  virtual void save() {
    SetColorArg(menu_current_blade, menu_current_arg, ShowColorStyle::getColor());
  }

  void mode_activate(bool onreturn) override {
    if (!onreturn) {
      load();
      show_color_all_.Start();
    }
    ColorSelectList<SPEC, ColorSelectMode<SPEC>>::mode_activate(onreturn);
  }

  void mode_deactivate() override {
    show_color_all_.Stop();
    ColorSelectList<SPEC, ColorSelectMode<SPEC>>::mode_deactivate();
  }

  ShowColorAllBladesTemplate<ShowColorStyle> show_color_all_;
};

}  // namespace mode

#endif
