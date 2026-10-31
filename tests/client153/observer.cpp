#include "Windows.h"
#include "ec_observer.h"
#include <stdexcept>

struct Model : CECObservable<Model> {};
struct Observer : CECObserver<Model> {
  int calls = 0;
  int removals = 0;
  void OnModelChange(const Model *, const CECObservableChange *) override { ++calls; }
  void OnUnregister(const Model *) override { ++removals; }
};

int main() {
  Model model;
  Observer once;
  Observer persistent;
  if (!model.RegisterObserver(&once, 1) || !model.RegisterObserver(&persistent))
    throw std::runtime_error("registration");
  CECObservableChange change;
  model.NotifyObservers(&change);
  model.NotifyObservers(&change);
  if (once.calls != 1 || once.removals != 1 || persistent.calls != 2)
    throw std::runtime_error("notifications");
  if (!model.UnregisterObserver(&persistent) || persistent.removals != 1)
    throw std::runtime_error("unregister");
}
