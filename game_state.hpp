#pragma once

#include <array>
#include <exception>
#include <optional>

class InvalidOperation : public std::exception {
public:
    const char* what() const noexcept override {
        return "invalid operation";
    }
};

struct PlayInfo {
    int dummyCount = 0;
    int magnifierCount = 0;
    int converterCount = 0;
    int cageCount = 0;
};

class GameState {
public:
    enum class BulletType { Live, Blank };
    enum class ItemType { Dummy, Magnifier, Converter, Cage };

    GameState() {
        hp[0] = 5;
        hp[1] = 5;
        currentPlayer = 0;
        liveCount = 0;
        blankCount = 0;
        knownTop = std::nullopt;
        cageActive = false;
        cageUsedThisTurn = false;
    }

    void fireAtOpponent(BulletType topBulletBeforeAction) {
        consumeBullet(topBulletBeforeAction);
        if (topBulletBeforeAction == BulletType::Live) {
            hp[1 - currentPlayer]--;
        }
        endTurnAttempt();
    }

    void fireAtSelf(BulletType topBulletBeforeAction) {
        consumeBullet(topBulletBeforeAction);
        if (topBulletBeforeAction == BulletType::Live) {
            hp[currentPlayer]--;
            endTurnAttempt();
        } else {
            // turn continues
        }
    }

    void useDummy(BulletType topBulletBeforeUse) {
        if (items[currentPlayer].dummyCount <= 0) throw InvalidOperation();
        items[currentPlayer].dummyCount--;
        consumeBullet(topBulletBeforeUse);
    }

    void useMagnifier(BulletType topBulletBeforeUse) {
        if (items[currentPlayer].magnifierCount <= 0) throw InvalidOperation();
        items[currentPlayer].magnifierCount--;
        knownTop = topBulletBeforeUse;
    }

    void useConverter(BulletType topBulletBeforeUse) {
        if (items[currentPlayer].converterCount <= 0) throw InvalidOperation();
        items[currentPlayer].converterCount--;
        
        if (topBulletBeforeUse == BulletType::Live) {
            liveCount--;
            blankCount++;
            knownTop = BulletType::Blank;
        } else {
            blankCount--;
            liveCount++;
            knownTop = BulletType::Live;
        }
    }

    void useCage() {
        if (items[currentPlayer].cageCount <= 0) throw InvalidOperation();
        if (cageUsedThisTurn) throw InvalidOperation();
        items[currentPlayer].cageCount--;
        cageUsedThisTurn = true;
        cageActive = true;
    }

    void reloadBullets(int liveCount, int blankCount) {
        this->liveCount = liveCount;
        this->blankCount = blankCount;
        this->knownTop = std::nullopt;
    }

    void reloadItem(int playerId, ItemType item) {
        if (item == ItemType::Dummy) items[playerId].dummyCount++;
        else if (item == ItemType::Magnifier) items[playerId].magnifierCount++;
        else if (item == ItemType::Converter) items[playerId].converterCount++;
        else if (item == ItemType::Cage) items[playerId].cageCount++;
    }

    double nextLiveBulletProbability() const {
        if (knownTop.has_value()) {
            return knownTop.value() == BulletType::Live ? 1.0 : 0.0;
        }
        if (liveCount + blankCount == 0) return 0.0;
        return static_cast<double>(liveCount) / (liveCount + blankCount);
    }

    double nextBlankBulletProbability() const {
        if (knownTop.has_value()) {
            return knownTop.value() == BulletType::Blank ? 1.0 : 0.0;
        }
        if (liveCount + blankCount == 0) return 0.0;
        return static_cast<double>(blankCount) / (liveCount + blankCount);
    }

    int winnerId() const {
        if (hp[0] <= 0) return 1;
        if (hp[1] <= 0) return 0;
        return -1;        
    }

private:
    int hp[2];
    PlayInfo items[2];
    int currentPlayer;
    int liveCount;
    int blankCount;
    std::optional<BulletType> knownTop;
    bool cageActive;
    bool cageUsedThisTurn;

    void consumeBullet(BulletType b) {
        if (b == BulletType::Live) {
            liveCount--;
        } else {
            blankCount--;
        }
        knownTop = std::nullopt;
    }

    void endTurnAttempt() {
        if (cageActive) {
            cageActive = false;
        } else {
            currentPlayer = 1 - currentPlayer;
            cageUsedThisTurn = false;
            cageActive = false;
        }
    }
};