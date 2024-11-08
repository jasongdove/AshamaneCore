/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LootPackets.h"
#include "Loot.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Loot::LootItemData const& lootItem)
{
    data.WriteBits(lootItem.Type, 2);
    data.WriteBits(lootItem.UIType, 3);
    data.WriteBit(lootItem.CanTradeToTapList);
    data.FlushBits();
    data << lootItem.Loot; // WorldPackets::Item::ItemInstance
    data << uint32(lootItem.Quantity);
    data << uint8(lootItem.LootItemType);
    data << uint8(lootItem.LootListID);
    return data;
}

void WorldPackets::Loot::LootUnit::Read()
{
    _worldPacket >> Unit;
}

WorldPacket const* WorldPackets::Loot::LootResponse::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;
    _worldPacket << uint8(FailureReason);
    _worldPacket << uint8(AcquireReason);
    _worldPacket << uint8(_LootMethod);
    _worldPacket << uint8(Threshold);
    _worldPacket << uint32(Coins);
    _worldPacket << uint32(Items.size());
    _worldPacket << uint32(Currencies.size());
    _worldPacket.WriteBit(Acquired);
    _worldPacket.WriteBit(AELooting);
    _worldPacket.FlushBits();

    for (LootItemData const& item : Items)
        _worldPacket << item;

    for (LootCurrency const& currency : Currencies)
    {
        _worldPacket << uint32(currency.CurrencyID);
        _worldPacket << uint32(currency.Quantity);
        _worldPacket << uint8(currency.LootListID);
        _worldPacket.WriteBits(currency.UIType, 3);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

void WorldPackets::Loot::LootItem::Read()
{
    uint32 Count;
    _worldPacket >> Count;

    Loot.resize(Count);
    for (uint32 i = 0; i < Count; ++i)
    {
        _worldPacket >> Loot[i].Object;
        _worldPacket >> Loot[i].LootListID;
    }
}

WorldPacket const* WorldPackets::Loot::LootRemoved::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;
    _worldPacket << LootListID;

    return &_worldPacket;
}

void WorldPackets::Loot::LootRelease::Read()
{
    _worldPacket >> Unit;
}

WorldPacket const* WorldPackets::Loot::LootMoneyNotify::Write()
{
    _worldPacket << Money;
    _worldPacket.WriteBit(SoleLooter);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::CoinRemoved::Write()
{
    _worldPacket << LootObj;

    return &_worldPacket;
}

void WorldPackets::Loot::LootRoll::Read()
{
    _worldPacket >> LootObj;
    _worldPacket >> LootListID;
    _worldPacket >> RollType;
}

WorldPacket const* WorldPackets::Loot::LootReleaseResponse::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Owner;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootList::Write()
{
    _worldPacket << Owner;
    _worldPacket << LootObj;

    _worldPacket.WriteBit(Master.has_value());
    _worldPacket.WriteBit(RoundRobinWinner.has_value());

    _worldPacket.FlushBits();

    if (Master)
        _worldPacket << *Master;

    if (RoundRobinWinner)
        _worldPacket << *RoundRobinWinner;

    return &_worldPacket;
}

void WorldPackets::Loot::SetLootSpecialization::Read()
{
    _worldPacket >> SpecID;
}

WorldPacket const* WorldPackets::Loot::StartLootRoll::Write()
{
    _worldPacket << LootObj;
    _worldPacket << int32(MapID);
    _worldPacket << uint32(RollTime);
    _worldPacket << uint8(ValidRolls);
    _worldPacket << uint8(Method);
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollBroadcast::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Player;
    _worldPacket << int32(Roll);
    _worldPacket << uint8(RollType);
    _worldPacket << Item;
    _worldPacket.WriteBit(Autopassed);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollWon::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Winner;
    _worldPacket << int32(Roll);
    _worldPacket << uint8(RollType);
    _worldPacket << Item;
    _worldPacket.WriteBit(MainSpec);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootAllPassed::Write()
{
    _worldPacket << LootObj;
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::LootRollsComplete::Write()
{
    _worldPacket << LootObj;
    _worldPacket << uint8(LootListID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::AELootTargets::Write()
{
    _worldPacket << uint32(Count);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Loot::DisplayToast::Write()
{
    _worldPacket << Quantity;
    _worldPacket << ToastMethod;
    _worldPacket << QuestID;

    _worldPacket.WriteBit(IsBonusRoll);

    _worldPacket.WriteBits(ToastType, 2);

    if (ToastType == TOAST_ITEM)
    {
        _worldPacket.WriteBit(Mailed);
        _worldPacket.FlushBits();

        // item instance
        bool hasItemBonus = !bonusListIDs.empty();
        _worldPacket << EntityId;
        _worldPacket << uint32(0); // RandomPropertiesSeed
        _worldPacket << uint32(RandomPropertiesID);
        _worldPacket.WriteBit(hasItemBonus);
        _worldPacket.WriteBit(false); // HasModifications
        if (hasItemBonus)
        {
            _worldPacket << uint8(1); // Indexes (works in case of 1 bonus, possibly should be bit mask of indexes?)

            uint32 bonusCount = bonusListIDs.size();
            _worldPacket << uint32(bonusCount);
            for (uint32 j = 0; j < bonusCount; ++j)
                _worldPacket << uint32(bonusListIDs[j]);
        }

        _worldPacket.FlushBits();

        _worldPacket << uint32(0); // SpecializationID
        _worldPacket << uint32(0);
    }
    else if (ToastType == TOAST_CURRENCY)
    {
        _worldPacket.FlushBits();
        _worldPacket << EntityId;
    }
    else
        _worldPacket.FlushBits();

    return &_worldPacket;
}
