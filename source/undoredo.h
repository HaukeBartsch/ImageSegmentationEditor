#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <boost/dynamic_bitset.hpp>

#include "volume.h"

///
/// \brief The UndoRedo class provides a time stream of snapshots of either volume,
/// highlight buffer or both objects
///
class UndoRedo
{
    public:
        static UndoRedo& getInstance()
        {
            static UndoRedo instance;
            return instance;
        }
        enum type {
          BUFFER=0,
          VOLUME,
          BOTH
        };

        // we can have changes on volumes and changes on buffers all need to go into the undo chain
        void add(Volume *vol) { // add a volume to undo
          // make a copy of the volume and add to
          if (!vol)
            return;
          Volume *volCopy = vol->duplicate();
          undoVolumes.push_back(volCopy);
          std::vector<int> *t = new std::vector<int>();
          // find the closest buffer backwards in time and add that one
          int tmp = currentStep;
          int bufferIdx = -1;
          for (; tmp >= 0; tmp--) {
            std::vector<int> *t = undoList.at(tmp);
            if (t->at(0) == BUFFER || t->at(0) == BOTH) {
              bufferIdx = t->at(1);
              break;
            }
          }
          if (bufferIdx != -1) {
            t->push_back(BOTH);
            t->push_back(undoVolumes.size()-1);
            t->push_back(bufferIdx);
            fprintf(stderr, "val: %d and %d", (int)undoVolumes.size()-1, (int)bufferIdx);
          } else { // should not happen
            t->push_back(VOLUME);
            t->push_back(undoVolumes.size()-1);
            t->push_back(0);
          }
          undoList.push_back( t );
          currentStep = undoList.size()-1;
        }
        void add(boost::dynamic_bitset<> *buffer) { // add a buffer to undo
          if (!buffer)
            return;
          boost::dynamic_bitset<> *bufCopy = new boost::dynamic_bitset<>(*buffer); // should make a copy of the buffer
          undoBuffer.push_back(bufCopy);
          std::vector<int> *t = new std::vector<int>();

          // find the closest volume backwards in time and add that one
          int tmp = currentStep;
          int volIdx = -1;
          for (; tmp >= 0; tmp--) {
            std::vector<int> *t = undoList.at(tmp);
            if (t->at(0) == VOLUME) {
              volIdx = t->at(1);
              break;
            }
            if (t->at(0) == BOTH) {
              volIdx = t->at(2);
              break;
            }
          }
          if (volIdx != -1) {
            t->push_back(BOTH);
            t->push_back(undoBuffer.size()-1);
            t->push_back(volIdx);
            fprintf(stderr, "val: %d and %d", (int)volIdx, (int)undoBuffer.size()-1);
          } else {
            t->push_back(BUFFER);
            t->push_back(undoBuffer.size()-1);
            t->push_back(0);
          }
          undoList.push_back( t );
          currentStep = undoList.size()-1;
        }
        void add(boost::dynamic_bitset<> *buffer, Volume *vol) { // add a buffer to undo
          if (!buffer || !vol)
            return;
          boost::dynamic_bitset<> *bufCopy = new boost::dynamic_bitset<>(*buffer); // should make a copy of the buffer
          undoBuffer.push_back(bufCopy);
          Volume *volCopy = vol->duplicate();
          undoVolumes.push_back(volCopy);
          std::vector<int> *t = new std::vector<int>();
          t->push_back(BOTH);
          t->push_back(undoBuffer.size()-1);
          t->push_back(undoVolumes.size()-1);
          fprintf(stderr, "val: %d and %d", (int)undoVolumes.size()-1, (int)undoBuffer.size()-1);
          undoList.push_back( t );
          currentStep = undoList.size()-1;
        }
        bool next() { // switch the current pointer to the next step
          if (currentStep < (int)undoList.size()-2) {
            currentStep++;
            fprintf(stderr, "currentStep is: %d\n", currentStep);
            return true;
          }
          return false;
        }
        bool prev() { // switch the current pointer to the previous step
          if (currentStep > 0) {
            currentStep--;
            fprintf(stderr, "currentStep is: %d\n", currentStep);
            return true;
          }
          return false;
        }
        bool isVolume() { // is the current step a volume?
          if (currentStep < 0 || currentStep > (int)undoList.size()-1)
            return false;

          std::vector<int> *t = undoList.at(currentStep);
          if (t->at(0) == VOLUME || t->at(0) == BOTH) { // if 1 its a volume
            return true;
          }
          return false;
        }
        bool isBuffer() { // is the current step a buffer?
          if (currentStep < 0 || currentStep > (int)undoList.size()-1)
            return false;

          std::vector<int> *t = undoList.at(currentStep);
          if (t->at(0) == BUFFER || t->at(0) == BOTH) { // if 1 its a volume
            return true;
          }
          return false;
        }
        bool isBoth() {
          if (currentStep < 0 || currentStep > (int)undoList.size()-1)
            return false;

          std::vector<int> *t = undoList.at(currentStep);
          if (t->at(0) == BOTH) { // if 1 its a volume
            return true;
          }
          return false;
        }

        Volume *getVolume() {
          if (currentStep < 0 || currentStep > (int)undoList.size()-1)
            return NULL; // nothing to return
          if (isBoth()) {
            int i = undoList.at(currentStep)->at(2);
            Volume *vol = undoVolumes.at(i);
            return vol;
          } else if (isVolume()) { // not a volume here
            int i = undoList.at(currentStep)->at(1);
            Volume *vol = undoVolumes.at(i);
            return vol;
          }
          return NULL;
        }
        boost::dynamic_bitset<> *getBuffer() {
          if (currentStep < 0 || currentStep > (int)undoList.size()-1)
            return NULL; // nothing to return
          if (isBuffer() || isBoth()) // not a buffer here
            return undoBuffer.at( undoList.at(currentStep)->at(1) );
          return NULL;
        }

        int maxUndoSteps;

    private:
        UndoRedo() { currentStep = -1; maxUndoSteps = -1; };// Constructor? (the {} brackets) are needed here.
        UndoRedo(UndoRedo const&);       // Don't Implement
        void operator=(UndoRedo const&); // Don't implement

        std::vector< Volume * > undoVolumes; // make a copy of the volume to allow undo
        std::vector< boost::dynamic_bitset<> *> undoBuffer;
        std::vector< std::vector<int>* > undoList; // store what the next undo would be
        int currentStep; // undefined
};


#endif // UNDOREDO_H
