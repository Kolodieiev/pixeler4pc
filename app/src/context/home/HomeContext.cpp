#include "HomeContext.h"

#include "../WidgetCreator.h"
#include "pixeler/manager/SettingsManager.h"
#include "pixeler/util/img/BmpUtil.h"

#define UPD_DISPLAY_INTERVAL_MS 5000UL

HomeContext::HomeContext()
{
  WidgetCreator creator;

  EmptyLayout* layout = creator.getEmptyLayout();
  setLayout(layout);

  String walpp_path = SettingsManager::get(STR_WALLPP_FILENAME);

  if (!walpp_path.isEmpty())
  {
    ImgData bmp = BmpUtil::loadBmp(walpp_path.c_str());

    if (bmp.data_ptr)
    {
      Image* wallpp_img = new Image(ID_WALLPAPER);
      layout->addWidget(wallpp_img);
      _wallpaper_ptr = bmp.data_ptr;
      wallpp_img->setWidth(bmp.width);
      wallpp_img->setHeight(bmp.height);
      wallpp_img->setSrc(_wallpaper_ptr);
    }
  }

  Label* lbl = new Label(ID_MESSAGE_LBL);
  layout->addWidget(lbl);
  lbl->setText("Натисни Enter");
  lbl->setWidth(TFT_WIDTH);
  lbl->setHeight(TFT_HEIGHT);
  lbl->setGravity(IWidget::GRAVITY_CENTER);
  lbl->setAlign(IWidget::ALIGN_CENTER);
  lbl->setTextSize(2);
  lbl->setTextColor(COLOR_CYAN);
  lbl->setTransparency(true);
}

HomeContext::~HomeContext()
{
  free(_wallpaper_ptr);
}

bool HomeContext::loop()
{
  return true;
}

void HomeContext::update()
{
  if (_input.isReleased(BtnID::BTN_OK))
  {
    _input.lock(BtnID::BTN_OK, CLICK_LOCK);
    openContextByID(ID_CONTEXT_MENU);
  }
}
