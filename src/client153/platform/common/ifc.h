/*
 * ifc.h — Immersion TouchSense (IFC) browser-build boundary (seam header).
 *
 * AMImmWrapper.cpp wraps Immersion Corporation's Immersion Foundation
 * Classes (force feedback for gamepads/joysticks). The pinned tree ships
 * the wrapper and AMImmEffect.h but NOT the IFC SDK headers or library —
 * the SDK is proprietary middleware, and force feedback cannot exist in a
 * browser sandbox anyway.
 *
 * This header declares the minimal class surface the wrapper references;
 * thirdparty/immersion/ifc_stub.cpp defines it with explicit failure
 * semantics (device creation fails, so the wrapper's own Init error path
 * runs and force feedback stays off).
 *
 * Declared limitation (never silent): the browser build has no force
 * feedback until a browser gamepad-vibration adapter implements this
 * wrapper's contract.
 */

#ifndef PW153_PLATFORM_IFC_H
#define PW153_PLATFORM_IFC_H

#include "Windows.h"

class CImmDevice {
 public:
  static CImmDevice *CreateDevice(HMODULE module, HWND window);
};

class CImmCompoundEffect {
 public:
  virtual ~CImmCompoundEffect();
};

class CImmProject {
 public:
  CImmProject();
  ~CImmProject();
  BOOL OpenFile(const char *projectName, CImmDevice *device);
  void Close();
  CImmCompoundEffect *CreateEffect(const char *effectName,
                                    CImmDevice *device);
  void DestroyEffect(CImmCompoundEffect *effect);
  BOOL Start(const char *effectName);
};

#endif  // PW153_PLATFORM_IFC_H
