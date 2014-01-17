/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2014 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Ericsson AB
 *
 */
#ifndef DB_TEMPLATE_H
#define	DB_TEMPLATE_H

#include <map>
#include <string>
#include "ncsgl_defs.h"

template <typename T>
class AmfDb {
  public:
   unsigned int insert(T *obj);
   void erase(T *obj);
   T *find(const SaNameT *name);
   
   typedef std::map<std::string, T*> AmfDbMap;
   typedef typename AmfDbMap::const_iterator const_iterator;

   const_iterator begin() const {return db.begin();}
   const_iterator end() const {return db.end();}

  private:
   AmfDbMap db;
};

template <typename T>
unsigned int AmfDb<T>::insert(T *obj) {
  osafassert(obj);
  std::string name((const char*)obj->name.value, obj->name.length);
  if (db.insert(std::make_pair(name, obj)).second) {
    return NCSCC_RC_SUCCESS;
  }
   else {
      return NCSCC_RC_FAILURE; // Duplicate
    }
 }

template <typename T>
void AmfDb<T>::erase(T *obj) {
  osafassert(obj);
  std::string name((const char*)obj->name.value, obj->name.length);
  db.erase(name);
}

template <typename T>
T *AmfDb<T>::find(const SaNameT *dn) {
  osafassert(dn);
  std::string name((const char*)dn->value, dn->length);
  typename AmfDbMap::iterator it = db.find(name);
  if (it == db.end())
    return NULL;
  else
    return it->second;
}

#endif	/* DB_TEMPLATE_H */
