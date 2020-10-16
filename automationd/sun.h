/*
 * sun.h
 *
 *  Created on: Mar 13, 2015
 *      Author: Marc
 */

#ifndef AUTOMATIOND_SUN_H_
#define AUTOMATIOND_SUN_H_

class Sun {
  public:
    Sun(const double& latitude, const double& longitude);
    virtual ~Sun();
    double get_elevation();
  private:
    double latitude_;
    double longitude_;
};

#endif /* AUTOMATIOND_SUN_H_ */
