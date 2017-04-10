/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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
 * Author(s): Genband
 *
 */

/*****************************************************************************
 *   FILE NAME: plms_scale.h
 *
 *   DESCRIPTION: C++ implementation of EE scale
 *
 ****************************************************************************/
#ifndef PLM_COMMON_PLMS_SCALE_H_
#define PLM_COMMON_PLMS_SCALE_H_

#include <condition_variable>
#include <mutex>
#include <set>
#include <string>
#include <thread>

class PlmsScaleThread : public std::thread {
 public:
  struct Node {
    std::string eeName;
  };

  explicit PlmsScaleThread(const std::string& script);
  ~PlmsScaleThread(void);

  void add(const Node);

 private:
  struct ltNode {
    bool operator()(const Node& n1, const Node& n2) const {
      return n1.eeName < n2.eeName;
    }
  };

  typedef std::set<Node, ltNode> NodeList;

  std::string scaleOutFile;

  NodeList nodeList;

  std::condition_variable nodeListCv;
  std::mutex nodeListMutex;

  static void _main(PlmsScaleThread* self) { self->main(); }

  void main(void);
  void executeScaleScript(int argc, char** argv);
};

#endif  // PLM_COMMON_PLMS_SCALE_H_
