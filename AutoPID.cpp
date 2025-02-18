#include "AutoPID.h"

AutoPID::AutoPID( double* input, double* setpoint, double* output, double outputMin, double outputMax,
                  double Kp, double Ki, double Kd ) {
  _input = input;
  _setpoint = setpoint;
  _output = output;
  _outputMin = outputMin;
  _outputMax = outputMax;
  setGains( Kp, Ki, Kd );
  _timeStep = 1000;
  _stopped = true;
}//AutoPID::AutoPID

void AutoPID::setGains( double Kp, double Ki, double Kd ) {
  _Kp = Kp;
  _Ki = Ki;
  _Kd = Kd;
}//AutoPID::setControllerParams

void AutoPID::setBangBang( double bangOn, double bangOff ) {
  _bangOn = bangOn;
  _bangOff = bangOff;
}//void AutoPID::setBangBang

void AutoPID::setBangBang( double bangRange ) {
  setBangBang( bangRange, bangRange );
}//void AutoPID::setBangBang

void AutoPID::setOutputRange( double outputMin, double outputMax ) {
  _outputMin = outputMin;
  _outputMax = outputMax;
}//void AutoPID::setOutputRange

void AutoPID::setTimeStep( unsigned long timeStep ) {
  _timeStep = timeStep;
}


bool AutoPID::atSetPoint( double threshold ) {
  return abs( *_setpoint - *_input ) <= threshold;
}//bool AutoPID::atSetPoint

void AutoPID::run() {
  if ( _stopped ) {
    _stopped = false;
    reset();
  }

  //if bang thresholds are defined and we're outside of them, use bang-bang control
  double absError = *_setpoint - *_input;

  if ( absError < 0 ) {
    absError *= -1;
  }

  if ( _bangOn && ( absError > _bangOn ) ) {
    if ( *_setpoint - *_input > 0 ) {
      *_output = _outputMax;
    } else {
      *_output = _outputMin;
    }

    // reset the integral and derivative part
    reset();
//     _lastStep = millis();
  } else if ( _bangOff && ( absError < _bangOff ) ) {
    *_output = 0;
    // reset the integral and derivative part
    reset();
//     _lastStep = millis();
  } else {                                    //otherwise use PID control
    unsigned long _dT = millis() - _lastStep;   //calculate time since last update

    if ( _dT >= _timeStep ) {                   //if long enough, do PID calculations
      _lastStep = millis();
      double _error = *_setpoint - *_input;
      _integral += ( _error * _Ki ) * _dT / 1000.0; // standard integral
      //_integral = constrain(_integral, _outputMin/_Ki, _outputMax/_Ki);
      double _dError = ( _error - _previousError ) / _dT / 1000.0; //derivative
      _previousError = _error;
      double pid = ( _Kp * _error ) + ( _integral ) + ( _Kd * _dError );
      //*_output = _outputMin + (constrain(PID, 0, 1) * (_outputMax - _outputMin));
      if ( pid < _outputMin ) {
        pid = _outputMin;
      }

      if ( pid > _outputMax ) {
        pid = _outputMax;
      }

      *_output = pid;
    }
  }
}//void AutoPID::run

void AutoPID::stop() {
  _stopped = true;
  reset();
}
void AutoPID::reset() {
  _lastStep = millis();
  _integral = 0;
  _previousError = 0;
}

bool AutoPID::isStopped() {
  return _stopped;
}

double AutoPID::getIntegral() {
  return _integral;
}

void AutoPID::setIntegral( double integral ) {
  _integral = integral;
}

void AutoPIDRelay::run() {
  AutoPID::run();

  while ( ( millis() - _lastPulseTime ) > _pulseWidth ) _lastPulseTime += _pulseWidth;

  *_relayState = ( ( millis() - _lastPulseTime ) < ( _pulseValue * _pulseWidth ) );
}


double AutoPIDRelay::getPulseValue() {
  return ( isStopped() ? 0 : _pulseValue );
}




