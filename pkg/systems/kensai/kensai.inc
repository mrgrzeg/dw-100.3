//-----------------------------------------------------------------------------

function CheckKensaiEquip(who, Item)

  var ItemCfg := ReadConfigFile(ITEMS_CFG);
  var AllowedItems := array{0x13C6, 0x13D5, 0x170B, 0x1711, 0x170D, 0x170F, 0x2796, 0x2FC4};

  if(Item.isa(POLCLASS_WEAPON))

    if(!ItemCfg[Item.objtype].Kensai and !ItemCfg[Item.objtype].NoCombatSkillGain)
      //SendSysMessage(who, "Nie mozesz zalozyc tej broni.", FONT_NORMAL, COLOR_RED);
      //return 0;
    endif

  elseif(Item.isa(POLCLASS_ARMOR))

    if(ItemCfg[Item.objtype].Shield)
      SendSysMessage(who, "Nie mozesz zalozyc tarczy.", FONT_NORMAL, COLOR_RED);
      return 0;
    elseif(!ItemCfg[Item.objtype].Cloth and !ItemCfg[Item.objtype].Kensai and !(Item.objtype in AllowedItems))
      if(Item.objtype != 0x204E) // deathshroud
        SendSysMessage(who, "Nie mozesz zalozyc tej zbroi.", FONT_NORMAL, COLOR_RED);
        return 0;
      endif
    endif

  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------
