#pragma once
#include <QString>
#include "TIGLCreatorDocument.h"

void handleDrawOption(const QString& actionName);

struct DrawOptionAction {
    QString label; // User-facing label
    std::function<void(TIGLCreatorDocument*, const QString& uid)> handler;
};

const std::vector<DrawOptionAction>& getWingDrawOptionsActions();
const std::vector<DrawOptionAction>& getFuselageDrawOptionsActions();
const std::vector<DrawOptionAction>& getPlaneDrawOptionsActions();
const std::vector<DrawOptionAction>& getRotorBladeDrawOptionsActions();
const std::vector<DrawOptionAction>& getRotorDrawOptionsActions();