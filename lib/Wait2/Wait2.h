#ifndef WAIT2_H
#define WAIT2_H

class Wait2 {
  private:
    unsigned long priortime;
    unsigned long currenttime;
    unsigned long delaytime;
    unsigned long counter;
  public:
  Wait2 (unsigned long x);
	bool ok_togo ();
};

   Wait2::Wait2 (unsigned long x) {   // use a constructor
      delaytime = x; 
      currenttime = millis();
      priortime = currenttime;
      //counter = 0;
    }   //  end set_value
	
   bool Wait2::ok_togo () {
      currenttime = millis();
      if (currenttime - priortime >= delaytime)
          {priortime = currenttime; return (true);}
          //{priortime = currenttime; Serial.print("Loops "); Serial.println(counter); counter =0; return (true);}
      //else {counter++; return(false);}
      else {return(false);}
    }  // end ok_togo

#endif