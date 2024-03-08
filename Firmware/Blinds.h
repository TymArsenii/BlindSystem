int window_shade_state[10];
int target_shade_state[10];
int curr_shade_state[10];
int window_shade_id;

struct WindowShade : Service::WindowCovering
{
  SpanCharacteristic *current;
  SpanCharacteristic *target;
  int shade_id;

  WindowShade(int shade_id) : Service::WindowCovering()
  {
    this->shade_id = shade_id;
    //window_shade_id = this->shade_id;

    current = new Characteristic::CurrentPosition(0);  // Window Shades have positions that range from 0 (fully lowered) to 100 (fully raised)

    target = new Characteristic::TargetPosition(0);  // Window Shades have positions that range from 0 (fully lowered) to 100 (fully raised)
    target->setRange(0, 100, 10);

    Serial.print("Configuring Motorized Window Shade");
    Serial.print("\n");
  }

  boolean update()
  {
    //int tilt = Characteristic::CurrentTiltAngle->getVal();

    window_shade_state[shade_id] = target->getNewVal();
    Serial.print("Window id = ");
    Serial.println(shade_id);

    if (window_shade_state[window_shade_id] > current->getVal())
    {
      LOG1("Raising Shade\n");
    }
    else if (window_shade_state[window_shade_id] < current->getVal())
    {
      LOG1("Lowering Shade\n");
    }
    return (true);
  }

  void loop()
  {
    current->setVal(curr_shade_state[1]); //shade_id
    target->setVal(window_shade_state[shade_id]); //shade_id
  }
};
