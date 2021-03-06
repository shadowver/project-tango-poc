//
// Created by stetro on 09.02.16.
//

#include "tango-augmented-reality/reconstruction_octree.h"


namespace tango_augmented_reality {

    ReconstructionOcTree::ReconstructionOcTree(glm::vec3 position, float range, int depth) {
        position_ = position;
        range_ = range;
        halfRange_ = range / 2;
        depth_ = depth;
        children_ = (ReconstructionOcTree **) malloc(sizeof(ReconstructionOcTree *) * 8);
        for (int i = 0; i < 8; ++i) {
            is_available_[i] = false;
        }
        if (depth_ == 0) {
            reconstructor = new Reconstructor();
        }
    }

    int ReconstructionOcTree::getSize() {
        int size = 0;
        if (depth_ == 0) {
            size = reconstructor->getPointCount();
        } else {
            for (int i = 0; i < 8; ++i) {
                if (is_available_[i]) {
                    size += children_[i]->getSize();
                }
            }
        }
        return size;
    }

    void ReconstructionOcTree::addPoint(glm::vec3 point) {
        if (point.x < position_.x ||
            point.y < position_.y ||
            point.z < position_.z ||
            point.x > position_.x + range_ ||
            point.y > position_.y + range_ ||
            point.z > position_.z + range_) {
            LOGE("Out of range!");
            return;
        }
        updated = true;
        if (depth_ == 0) {
            reconstructor->addPoint(point);
        } else {
            int index = getChildIndex(point);
            if (!is_available_[index]) {
                initChild(point, index);
            }
            children_[index]->addPoint(point);
        }
    }

    int ReconstructionOcTree::getClusterCount() {
        if (depth_ != 0) {
            int size = 0;
            for (int i = 0; i < 8; ++i) {
                if (is_available_[i]) {
                    size += children_[i]->getClusterCount();
                }
            }
            return size;
        }
        return 1;
    }

    void ReconstructionOcTree::reconstruct() {
        if (depth_ == 0 && updated) {
            reconstructor->reconstruct();
        } else if (updated) {
            for (int i = 0; i < 8; ++i) {
                if (is_available_[i]) {
                    children_[i]->reconstruct();
                }
            }
        }
        updated = false;
    }

    std::vector <glm::vec3> ReconstructionOcTree::getMesh() {
        if (depth_ != 0) {
            std::vector <glm::vec3> mesh;
            for (int i = 0; i < 8; ++i) {
                if (is_available_[i]) {
                    std::vector <glm::vec3> childMesh = children_[i]->getMesh();
                    if (childMesh.size() > 0) {
                        mesh.insert(mesh.end(), childMesh.begin(), childMesh.end());
                    }
                }
            }
            return mesh;
        }
        reconstructor->clearPoints();
        return reconstructor->getMesh();
    }


    void ReconstructionOcTree::clear() {
        if (depth_ != 0) {
            for (int i = 0; i < 8; ++i) {
                if (is_available_[i]) {
                    children_[i]->clear();
                }
            }
        } else {
            reconstructor->reset();
        }
    }

    void ReconstructionOcTree::initChild(glm::vec3 location, int index) {
        glm::vec3 childPosition;
        childPosition.x = (location.x > (position_.x + halfRange_))
                          ? (position_.x + halfRange_) : (position_.x);
        childPosition.y = (location.y > (position_.y + halfRange_))
                          ? (position_.y + halfRange_) : (position_.y);
        childPosition.z = (location.z > (position_.z + halfRange_))
                          ? (position_.z + halfRange_) : (position_.z);
        children_[index] = new ReconstructionOcTree(childPosition, halfRange_, depth_ - 1);
        is_available_[index] = true;
    }


    int ReconstructionOcTree::getChildIndex(glm::vec3 point) {
        if (point.x < position_.x + halfRange_) {
            if (point.y < position_.y + halfRange_) {
                if (point.z < position_.z + halfRange_) { return 0; } else { return 1; }
            } else {
                if (point.z < position_.z + halfRange_) { return 2; } else { return 3; }
            }
        } else {
            if (point.y < position_.y + halfRange_) {
                if (point.z < position_.z + halfRange_) { return 4; } else { return 5; }
            } else {
                if (point.z < position_.z + halfRange_) { return 6; } else { return 7; }
            }
        }
    }

}