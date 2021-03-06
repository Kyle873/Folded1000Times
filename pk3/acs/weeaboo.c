#include "zcommon.acs"
#library "weeaboo"

#include "commonfuncs.h"

#include "weeb_const.h"
#include "weeb_frosthammer.h"
#include "weeb_joy.h"

global int 58:LevelCount;
//global int 59:RunningInZandro;
//global int 60:RunningInZDoom;

int playerTimers[PLAYERMAX][TIMER_COUNT];
int playerTimeFreeze[PLAYERMAX];
int ClientEnterLocks[PLAYERMAX];
int dodgeitem;
int IsServer;
int array_recoilrules[PLAYERMAX];
int array_autoswitch[PLAYERMAX];
int array_beepbeepbeep[PLAYERMAX];
int array_nopan[PLAYERMAX];
int array_noannounce[PLAYERMAX];
int flashlightOn[PLAYERMAX];
int NotADoomGame;

int GotShotgun;
int GotCarronade;
int GotUzi;
int GotHam; // HAM
int GotIronMaiden;
int GotLegion;
int GotFrosthammer;

int MusicRandomizerIsIncluded;

int IronMaidenMusic[IRONMUS] = 
{ "D_IRONM1","D_IRONM2","D_IRONM3","D_IRONM4","D_IRONM5","D_IRONM6","D_IRONM7" };

#include "weeb_auto.h"

script WEEB_RESPAWN respawn
{
    // Death forcefully takes away all the player's remaining tokens (if killed
    // via divine intervention or freak accident or whatever), so respawning
    // hands over a fresh batch of tokens.
    if (GameSkill () == 0) { GiveInventory("ContraLifeToken",10); }
    if (GameSkill () == 1) { GiveInventory("ContraLifeToken",8);  }
    if (GameSkill () == 2) { GiveInventory("ContraLifeToken",6); }
    if (GameSkill () == 3) { GiveInventory("ContraLifeToken",4); }
    if (GameSkill () == 4) { GiveInventory("ContraLifeToken",2); }
    // Takes away a million points on respawn.
    // If you have at least ten million points, take ten million.
    // If you have a hundred million points, take a hundred million.
    // There's a better way to do this, but not with Zandro's 32xxx integeter limit.
    if (CheckInventory("OkuPoints") >= 1) { TakeInventory("OkuPoints",1); }
    else if (CheckInventory("ManPoints") >= 1000) { TakeInventory("ManPoints",1000); }
    else if (CheckInventory("ManPoints") >= 100) { TakeInventory("ManPoints",100); }
    else { TakeInventory("ManPoints",99); TakeInventory("Points",9999); }

    if (GetCvar("dst_debug") == 1) { Log(s:"Player ", d:PlayerNumber(), s:" respawning, executing WEEB_ENTER."); }
    ACS_ExecuteAlways(WEEB_ENTER,0,0,0);
}

script WEEB_OPEN OPEN
{
    IsServer = 1;
    int s = unusedTID(37000, 47000);

    delay(1);
    if (GetCvar("dst_debug") == 1) { Log(s:"dst_runninginzandro is at ", d:GetCvar("dst_runninginzandro")); }
    if (GameType() != GAME_TITLE_MAP) { LevelCount++; } // Titlemaps add to the global variable, irritatingly.
    /*if (Spawn("Doomsphere",0,0,0,s)) // Attempt to spawn a Doomsphere, a Zandronum-exclusive item.
    {
        Thing_Remove(s);
        //RunningInZandro = 1;
        if (GetCvar("dst_debug") == 1) { Log(s:"Zandronum check successful."); }
        SetCVar("dst_runninginzandro",1);
        SetCVar("dst_runninginzdoom",0);
    }*/
    // This didn't work because the Spawn would choose to not work at various times, because of collision.
    // If there is no room for an actor to exist, it simply won't spawn.
    // To get around this, instead I'm doing it the opposite way, using SpawnForced to spawn a ZDoom-exclusive item.
    // This is not a future-proof method at all; if Zandronum catches up with ZDoom (hahahaha), it'll spawn this item as well and read as ZDoom.
    // If we ever reach that point and I am either dead or have moved on from Doom, simply replace the script with the uncommented above script and change it to SpawnForced.
    if (SpawnForced("SpeakerIcon",0,0,0,s,0))
    { // Other ZDoom-exclusive items include PowerBuddha (not in GZDoom 1.9), CajunTrace, CajunBodyNode, ArtiPoisonBagShooter, ArtiPoisonBagGiver, and AmbientSoundNoGravity.
        Thing_Remove(s);
        //RunningInZDoom = 1;
        if (GetCvar("dst_debug") == 1) { Log(s:"ZDoom check successful."); }
        SetCVar("dst_runninginzandro",0);
        SetCVar("dst_runninginzdoom",1);
    }
    else
    {
        if (GetCvar("dst_debug") == 1) { Log(s:"ZDoom check unsuccessful, assuming Zandronum run."); }
        SetCVar("dst_runninginzandro",1);
        SetCVar("dst_runninginzdoom",0);
    }
    delay(1);
    if (GetCvar("compat_clientssendfullbuttoninfo") == 0 && GetCvar("dst_runninginzandro") == 1 && !isSinglePlayer()) // Singleplayer already reads extra player input, no need to make sure it's on.
    {
        if (GetCvar("dst_debug") == 1) { Log(s:"compat_clientssendfullbuttoninfo is turned off. Automatically turning it on."); }
        ConsoleCommand("compat_clientssendfullbuttoninfo 1");
    }
}

script WEEB_OPEN_CLIENT OPEN clientside
{
    delay(1);
    // Not sure if there's a reason for this, now that Z& 2.0 is out and there's cvarinfo.
    // Hm.
}

script WEEB_DECORATE (int burrshet, int ballshat) // All the Decorate/utility/etc calls go here.
{
int i;
int GravityRoll;
int GravityOfLight;

    switch (burrshet)
    {
    case WEEB_DEC_DAMAGE:
        int delaytotal = 104;
        int delaytimer = delaytotal;
        int delaytime = 10;
        int delaycount = 0;

        SetPlayerProperty(0,0,PROP_TOTALLYFROZEN);

        // If a player somehow manages to get hurt through the Invulnerability,
        // don't take any health anyway.
        if (CheckInventory("PowerShieldProtection") == 1) { terminate; }
        
        //GiveInventory("PointsTookDamage",1);

        if( GetCVar( "dst_doomhealth" ) == 0 )
        {
            if (GetCvar("dst_nodamagepenalty") == 0)
            {
              if (CheckInventory("ContraArmorToken") >= 1)
                { TakeInventory("ContraArmorToken",1); TakeInventory("HyperComboCounter",25); }
              else if (CheckInventory("OverLifeToken") >= 1)
                { TakeInventory("OverLifeToken",1); TakeInventory("HyperComboCounter",50); }
              else if (CheckInventory("ContraLifeToken") >= 1)
                { TakeInventory("ContraLifeToken",1); TakeInventory("HyperComboCounter",50); }
            }
            else
            {
              if (CheckInventory("ContraArmorToken") >= 1)
                { TakeInventory("ContraArmorToken",1); }
              else if (CheckInventory("OverLifeToken") >= 1)
                { TakeInventory("OverLifeToken",1); }
              else if (CheckInventory("ContraLifeToken") >= 1)
                { TakeInventory("ContraLifeToken",1); }
            }

        GiveInventory("Wounded",1);
        
        SetActorProperty(0,APROP_INVULNERABLE,1);
        
        while (delaytimer > 0)
        {
            if (delaytimer % 2)
                SetActorProperty(0,APROP_RENDERSTYLE,STYLE_Fuzzy);
            else
                SetActorProperty(0,APROP_RENDERSTYLE,STYLE_Add);
            
            delaytimer--;
            delaycount++;
            if (delaycount > 1)
            {
                delaytime--;
                delaycount = 0;
            }
            
            Delay(delaytime);
        }

        TakeInventory("Wounded",1);
        
        SetActorProperty(0,APROP_RENDERSTYLE,STYLE_Normal);
        SetActorProperty(0,APROP_INVULNERABLE,0);
        }
        else
        { TakeInventory("HyperComboCounter",25); }
        break;

        // Done during the forward special. Turns the player into the same
        // species as the enemies so they can slip through them dramatically.
        // Applying +THRUACTORS would make them go through decorations.
    case WEEB_DEC_FREEZE:
        SetActorProperty(0,APROP_Species,"Body");
        SetPlayerProperty(0,1,PROP_TOTALLYFROZEN);
        break;

    case WEEB_DEC_UNFREEZE:
        if (GameType() == GAME_NET_DEATHMATCH) { SetActorProperty(0,APROP_Species,"DMPlayer"); }
        else { SetActorProperty(0,APROP_Species,"Player"); }
        SetPlayerProperty(0,0,PROP_TOTALLYFROZEN);
        break;

        // You know, I probably could consolidate all these checks into one smaller check.
    case WEEB_DEC_SHOTCHECK:
        if (isSinglePlayer() && GotShotgun == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

        // Meh.
    case WEEB_DEC_CARRCHECK:
        if (isSinglePlayer() && GotCarronade == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_UZICHECK:
        if (isSinglePlayer() && GotUzi == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case 6:
        GiveInventory("DodgeGhostOn",1);
        Delay(18);
        GiveInventory("DodgeGhostOff",1);
        break;

    case WEEB_DEC_MEGASPHERE:
        // That feel when your health system is so complicated that it's easier to do in ACS than Decorate.
        if (GameSkill () == 0)
        {
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 10) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          GiveInventory("ContraArmorToken",4);
        }
        if (GameSkill () == 1)
        {
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 8) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 4) { GiveInventory("OverLifeToken",1); }
          GiveInventory("ContraArmorToken",4);
        }
        if (GameSkill () == 2)
        {
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 6) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 3) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 3) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 3) { GiveInventory("OverLifeToken",1); }
          GiveInventory("ContraArmorToken",4);
        }
        if (GameSkill () == 3)
        {
          if (CheckInventory("ContraLifeToken") < 4) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 4) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 4) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 4) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 2) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 2) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
        }
        if (GameSkill () == 4)
        {
          if (CheckInventory("ContraLifeToken") < 2) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("ContraLifeToken") < 2) { GiveInventory("ContraLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 2) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("OverLifeToken") < 2) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
          if (CheckInventory("ContraArmorToken") < 3) { GiveInventory("ContraArmorToken",1); }
        }
        if (GameSkill () == 5)
        {
          if (CheckInventory("OverLifeToken") < 1) { GiveInventory("OverLifeToken",1); }
          if (CheckInventory("ContraArmorToken") < 2) { GiveInventory("ContraArmorToken",1); }
          if (CheckInventory("ContraArmorToken") < 2) { GiveInventory("ContraArmorToken",1); }
        }
        break;

    case WEEB_DEC_WEPSTAY:
        SetResultValue(GetCVar("sv_weaponstay"));
        break;

    case WEEB_DEC_HAMCHECK: // HAAAAAM
        if (isSinglePlayer() && GotHam == 1) { SetResultValue(1); } // HAAAAAAAAAM
        else { SetResultValue(0); } // HAAAAAAAAAAAAAAAAAAAAAAM
        break;

    case WEEB_DEC_HAMAMMO1: // HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM
        SetAmmoCapacity("HammerCharge",200);
        break;

    case WEEB_DEC_HAMAMMO2: // HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM
        SetInventory("HammerCharge",60);
        SetAmmoCapacity("HammerCharge",100);
        break;

    case WEEB_DEC_HAMAMMO3: // HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM
        SetInventory("HammerCharge",0);
        SetAmmoCapacity("HammerCharge",100);
        break;

        // Momentary body freeze when hitting with the hammer or Iron Fist.
    case WEEB_DEC_FREEZE2:
        SetPlayerProperty(0,1,PROP_TOTALLYFROZEN);
        break;

    case WEEB_DEC_UNFREEZE2:
        SetPlayerProperty(0,0,PROP_TOTALLYFROZEN);
        break;

        // Iron Maiden cancelling script.
    case WEEB_DEC_REMOVEKEBAB:
        FadeRange(255,255,255,1.00,0,0,0,0,1.50);
        TakeInventory("HenshinDeactivation",1);
        TakeInventory("InIronMaiden",1);
        TakeInventory("MaidenHalo",1);
        TakeInventory("Iron Fist",1);
        TakeInventory("IronMaidenSpeed",1);
        TakeInventory("IronMaidenSlowness",1);
        TakeInventory("IronMaidenSlowness2",1);
        TakeInventory("IronMaidenWarning",1);
        TakeInventory("IronMaidenProtection",1);
        TakeInventory("IronMaidenProtectionF",1);
        TakeInventory("IronMaidenProtectionD",1);
        TakeInventory("IronMaidenProtectionC",1);
        TakeInventory("IronMaidenProtectionB",1);
        TakeInventory("IronMaidenProtectionA",1);
        TakeInventory("IronMaidenProtectionS",1);
        TakeInventory("Armor",0x7FFFFFFF);
        TakeInventory("HenshinActivated",1);
        GiveInventory("IronMaidenVisionReset",1);
        GiveInventory("Kharon + Acacia A-22",1);
        GiveInventory("Kharon",1);
        SetActorProperty(0,APROP_JUMPZ,9.0);
        if (CheckInventory("GotLegion") == 1) { GiveInventory("Kharon + Legion",1); }
        if (CheckInventory("GotShotgun") == 1) { GiveInventory("Kharon + Testament",1); }
        if (CheckInventory("GotUzi") == 1) { GiveInventory("Kharon + Sabbath",1); }
        if (CheckInventory("GotCarronade") == 1) { GiveInventory("Kharon + Exodus",1); }
        if (CheckInventory("GotHammer") == 1) { GiveInventory("Kharon + Omen",1); }
        if (CheckInventory("GotFrosthammer") == 1) { GiveInventory("Kharon + Frosthammer",1); }
        SetWeapon("Kharon + Acacia A-22");
        ACS_ExecuteAlways(275,0,WEEB_DEC_CHANGEMUSBACK,0,0);
        delay(1);
        TakeInventory("IronMaidenArmor",0x7FFFFFFF);
        TakeInventory("IronMaidenArmor2",0x7FFFFFFF);
        break;

        // If player's playing on max difficulty, health items just don't spawn.
    case WEEB_DEC_DIFFCHECK:
        if (GameSkill () == 5) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_ONLINECHECK:
        // The Shotgun is never given out right away.
        // It's powerful as hell, so it needs to be staggered out a bit.
        // Also gives people opportunity to get used to the sword.
        // Also note that the Shotgun spawn is never given out online.
        // This might be a problem if people ever play Doom 1 mapsets online.
        if ( (GetCvar("dst_noshotgunlimiter") == 0 && LevelCount < 2) || GetCvar("dst_noshotgun") == 1 || !isSinglePlayer())
             { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_HENSHIN:
        delay(10);
        TakeInventory("HenshinActivated",1);
        break;

    case WEEB_DEC_ARMRCHECK:
        if (isSinglePlayer() && GotIronMaiden == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_BARRELSPAWN:
        SetResultValue(GetCVar("sv_barrelrespawn"));
        break;

    case WEEB_DEC_LEGIONCHECK:
        if (isSinglePlayer() && GotLegion == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_STWEAPCHECK:
        if (GetCvar("dst_arrogantweapons") == 1) { SetResultValue(1); }
        else if (GetCvar("dst_arrogantweapons") == 2) { SetResultValue(2); }
        /*switch (ballshat)
        {
        case 0:
          if (GetCvar("dst_arrogantweapons") == 2)
             { SetActorState(0,"SpawnTestament"); }
          else
             { SetActorState(0,"SpawnLegion"); }
          break;
        }*/
        break;

    case WEEB_DEC_FROSTCHECK:
        if (isSinglePlayer() && GotFrosthammer == 1) { SetResultValue(1); }
        else { SetResultValue(0); }
        break;

    case WEEB_DEC_COOKIEQUOTE:
        i = random(0, QUOTECOUNT-1);
        delay(35);
        LocalAmbientSound("health/cookieriff",127);
        HudMessage(s:CookieQuote[i];HUDMSG_FADEINOUT | HUDMSG_LOG,1423,CR_GOLD,0.5,0.2,5.25,0.5,0.5);
        break;

        // The Frosthammer's slowdown on monsters.
        // 50% of the current monster's speed. Stackable.
    case WEEB_DEC_SLOWDOWN:
        SetActorProperty(0,APROP_SPEED,(GetActorProperty(0,APROP_SPEED)*1/2));
        break;

    case WEEB_DEC_GUNSOULS:
        SetResultValue(GetCVar("dst_gunsouls"));
        break;
        
    case WEEB_DEC_DOOMHEALTH:
        SetResultValue(GetCVar("dst_doomhealth"));
        break;

        // Takes the player's existing gravity and reduces it a bit because of
        // all the spinning blades. That's how physics works, right?
    case WEEB_DEC_LEGSPECIAL:
        GravityRoll = GetActorProperty(0,APROP_Gravity);
        SetActorProperty(0,APROP_Gravity,GravityRoll - 0.15);
        delay(1);
        while (GravityOfLight < 1575)
        {
            if (isDead(0)) { terminate; }
            Delay(1);
            GravityOfLight++;
        }
        GravityRoll = GetActorProperty(0,APROP_Gravity);
        GravityOfLight = 0;
        delay(1);
        SetActorProperty(0,APROP_Gravity,GravityRoll + 0.15);
        TakeInventory("LegionSpecialCounter",1);
        break;

        // See above with WEEB_DEC_ONLINECHECK
    case WEEB_DEC_ONLINECHECK2:
        if ( (GetCvar("dst_nochaingunlimiter") == 0 && LevelCount < 4) || GetCvar("dst_nochaingun") == 1 || !isSinglePlayer())
             { SetResultValue(1); }
        else { SetResultValue(0); }
        break;
    }
}

script WEEB_CLIENTDECORATE (int boreshut, int bowlshot) clientside
{
    if (boreshut < 98 && ConsolePlayerNumber() != PlayerNumber()) { terminate; } 

    switch (boreshut)
    {
    case WEEB_DEC_GETMESSAGES:
        // OH COOL YOU CAN DO INTS WITHIN INTS
        // I should do this with the weapon pickup checks.
        switch (bowlshot)
        {
        case 0:
          Log(s:"\cfThe \cjTestament \cfshotgun. You may wield it...");
          break;

        case 1:
          Log(s:"\cfThe \cjExodus \cfgrenade launcher. You may wield it...");
          break;

        case 2:
          Log(s:"\cfThe \cjSabbath \cfsubmachine gun. You may wield it...");
          break;

        case 3:
          Log(s:"\cfThe \cjOmen \cfwarhammer. You may wield it...");
          break;

        case 4:
          Log(s:"\cfThe \cjIron Savior \cfarmor. You may now become the \cjIron Maiden\cf...\n\cu(Use Inventory to activate)");
          break;

        case 5:
          Log(s:"\ckThe \cbMaw of the Legion\ck. It screams for blood...");
          break;

        case 6:
          Log(s:"\ckThe \cbFrosthammer of Cocytus\ck. It screams for blood...");
          break;
        }
        break;

        // If the player has the Metal Jukebox loaded and the music on, there is
        // no music change. Otherwise, well, IRON MAIDEN MUSIC GOOOOOOO
    case WEEB_DEC_CHANGEMUS:
        if(CheckInventory("IAmAnAwesomePersonWhoLikesCoolMusic") == 1) 
        { if (getcvar("norandommusic") == 0) { terminate; }}
        int i = random(0, IRONMUS-1);
        if(getcvar("dst_cl_nomusic") == 0) { LocalSetMusic(IronMaidenMusic[i],0); }
        break;

    case WEEB_DEC_CHANGEMUSBACK:
        if(CheckInventory("IAmAnAwesomePersonWhoLikesCoolMusic") == 1)
        { if (getcvar("norandommusic") == 0) { terminate; }}
        if(getcvar("dst_cl_nomusic") == 0) { LocalSetMusic("*"); }
        break;

    //[Scroton] cases 98 and 99, here so they're not subject to the ConsolePlayerNumber check
    //if you want more cases that aren't subject to it, put them here in descending order
    //and decrease the number in the check at the top of this script

    case WEEB_DEC_TOASTER:
        if (GetCvar("dst_runninginzandro") == 1)
        {
            if(GetCvar("dst_cl_toaster") <= 0) { SetActorState(0,"Toaster0"); }
            if(GetCvar("dst_cl_toaster") == 1) { SetActorState(0,"Toaster1"); }
            if(GetCvar("dst_cl_toaster") >= 2) { SetActorState(0,"Toaster2"); }
        }
        else
        {
            if(GetCvar("dst_cl_toasterzdoom") <= 0) { SetActorState(0,"Toaster0"); }
            if(GetCvar("dst_cl_toasterzdoom") == 1) { SetActorState(0,"Toaster1"); }
            if(GetCvar("dst_cl_toasterzdoom") >= 2) { SetActorState(0,"Toaster2"); }
        }
        break;

    case WEEB_DEC_TOASTER2:
        SetResultValue(getCvar("dst_cl_toaster"));
        break;
    }
}

script WEEB_UNLOADING UNLOADING
{
    GiveInventory("ManPoints",1);
    if (GameSkill () == 1) { GiveInventory("Points",1000); }//Log(s:"Difficulty bonus: +1000 Points"); }
    if (GameSkill () == 2) { GiveInventory("Points",5000); }//Log(s:"Difficulty bonus: +5000 Points"); }
    if (GameSkill () == 3) { GiveInventory("ManPoints",1); }//Log(s:"Difficulty bonus: +10000 Points"); }
    if (GameSkill () == 4) { GiveInventory("ManPoints",5); }//Log(s:"Difficulty bonus: +50000 Points"); }
    if (GameSkill () == 5) { GiveInventory("ManPoints",10); }//Log(s:"Difficulty bonus: +100000 Points"); }
    //if (CheckInventory("PointsIsBrutal") == 1) { GiveInventory("Points",10000); Log(s:"A Devil in God's Country: +10000 Points"); } // Played with Brutal Doom or with the skeletons
    //if (CheckInventory("PointsTookDamage") == 0) { GiveInventory("Points",1000000); Log(s:"Iron Man: +1000000 Points"); } // No damage.
    //if (CheckInventory("PointsSpeedrunning") > 0) { GiveInventory("Points",1000000); Log(s:"Fast as a Shark: +1000000 Points"); } // Finished level within 25 seconds.
    //if (CheckInventory("PointsSpeedrunning") > 525) { GiveInventory("Points",1000000); Log(s:"Faster Than You'll Ever Live To Be: +1000000 Points"); } // Finished level within 10 seconds.
    //if (CheckInventory("PointsKilledMonsters") == 1) { GiveInventory("Points",1000000); Log(s:"Blood Junkie: +1000000 Points"); } // Killed all monsters.
    //if (CheckInventory("PointsFoundAllSecrets") == 1) { GiveInventory("Points",1000000); Log(s:"Sights Unseen: +1000000 Points"); } // Found all secrets.
    //if (CheckInventory("PointsNoSpecials") == 1) { GiveInventory("Points",100000); Log(s:"Angel of Mercy: +100000 Points"); } // No special moves.
    //if (CheckInventory("PointsNoAttacks") == 1) { GiveInventory("Points",1000000); Log(s:"Angels Don't Kill: +1000000 Points"); } // Never attacked.

    TakeInventory("AlreadyInLevel",1);
    //TakeInventory("PointsTookDamage",1);
    TakeInventory("PointsFoundAllSecrets",1);
    TakeInventory("PointsKilledMonsters",1);
    //TakeInventory("PointsSpeedrunning",0x7FFFFFFF);

    TakeInventory("MaidenHalo",1);

    TakeInventory("KharonSwung",1);
    TakeInventory("SlashingLikeAGaijin",1);
    TakeInventory("ShootingLikeABaka",1);
    TakeInventory("DoubleTapForward",1);
    TakeInventory("DoubleTapBack",1);
    TakeInventory("DoubleTapLeft",1);
    TakeInventory("DoubleTapRight",1);
    TakeInventory("Wounded",1);
    TakeInventory("SuperTimeFreezer",1);
    TakeInventory("HammerSpecialCounter",1);
    TakeInventory("HammerSpecialAmount",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyRight",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyForward",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyLeft",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyBack",0x7FFFFFFF);
    TakeInventory("HammerOverchargeLevel",0x7FFFFFFF);
    TakeInventory("EnviroDamageCount",0x7FFFFFFF);
    TakeInventory("EnviroDamageCooldown",0x7FFFFFFF);
    TakeInventory("StopTheBlock",0x7FFFFFFF);
    TakeInventory("BlockLife",0x7FFFFFFF);
    TakeInventory("GhostStepCooldown",0x7FFFFFFF);
    TakeInventory("DoubleTapCooldown",0x7FFFFFFF);
    TakeInventory("MidCombat",0x7FFFFFFF);
    TakeInventory("LegionCounter",0x7FFFFFFF);
    TakeInventory("LegionSpecialCounter",0x7FFFFFFF);
    TakeInventory("LegionStacked",0x7FFFFFFF);
    TakeInventory("SwordAttack",0x7FFFFFFF);
}

script WEEB_DEATH DEATH // Mostly the same, except for a few notable exclusions
{
    TakeInventory("PointsSpeedrunning",0x7FFFFFFF);

    TakeInventory("KharonSwung",1);
    TakeInventory("SlashingLikeAGaijin",1);
    TakeInventory("ShootingLikeABaka",1);
    TakeInventory("DoubleTapForward",1);
    TakeInventory("DoubleTapBack",1);
    TakeInventory("DoubleTapLeft",1);
    TakeInventory("DoubleTapRight",1);
    TakeInventory("Wounded",1);
    TakeInventory("SuperTimeFreezer",1);
    TakeInventory("HammerSpecialCounter",1);
    TakeInventory("HammerSpecialAmount",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyRight",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyForward",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyLeft",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyBack",0x7FFFFFFF);
    TakeInventory("HammerOverchargeLevel",0x7FFFFFFF);
    TakeInventory("EnviroDamageCount",0x7FFFFFFF);
    TakeInventory("EnviroDamageCooldown",0x7FFFFFFF);
    TakeInventory("StopTheBlock",0x7FFFFFFF);
    TakeInventory("BlockLife",0x7FFFFFFF);
    TakeInventory("GhostStepCooldown",0x7FFFFFFF);
    TakeInventory("DoubleTapCooldown",0x7FFFFFFF);
    TakeInventory("MidCombat",0x7FFFFFFF);
    TakeInventory("LegionCounter",0x7FFFFFFF);
    TakeInventory("LegionSpecialCounter",0x7FFFFFFF);
    TakeInventory("LegionStacked",0x7FFFFFFF);
    TakeInventory("SwordAttack",0x7FFFFFFF);
}

script WEEB_CREDITS (int changelogshit2) NET CLIENTSIDE
{
    switch (changelogshit2)
    {
    case 1:
        Log(s:DemonCredits);
        break;

    case 2:
        Log(s:DemonChangelog);
        break;
    }
}

script WEEB_PUKE2 (void) NET CLIENTSIDE
{
// I can't believe I'm dedicating an entire script to this one instance.

    if (flashlightOn[PlayerNumber()])
    {
        flashlightOn[PlayerNumber()] = 0;
        GiveInventory("FlashlightStopper",1);
        ActivatorSound("flashlight/off", 127);
    }
    else
    {
        flashlightOn[PlayerNumber()] = 1;
        ActivatorSound("flashlight/on", 127);
    }
}



script WEEB_PUKE3 (void) NET // why am i having to do separate scripts for this shit
{   // Emergency inventory reset.
    //if (ConsolePlayerNumber() != PlayerNumber()) { terminate; }
    
    delay(1);
    FadeRange(0,0,0,1.00,0,0,0,0,3.50);
    LocalAmbientSound("level/intro",127);
    //Print(s:"Attempting emergency inventory reset.");

    TakeInventory("KharonSwung",1);
    TakeInventory("SlashingLikeAGaijin",1);
    TakeInventory("ShootingLikeABaka",1);
    TakeInventory("DoubleTapForward",1);
    TakeInventory("DoubleTapBack",1);
    TakeInventory("DoubleTapLeft",1);
    TakeInventory("DoubleTapRight",1);
    TakeInventory("Wounded",1);
    TakeInventory("SuperTimeFreezer",1);
    TakeInventory("HammerSpecialCounter",1);
    TakeInventory("HammerSpecialAmount",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyRight",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyForward",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyLeft",0x7FFFFFFF);
    TakeInventory("DoubleTapReadyBack",0x7FFFFFFF);
    TakeInventory("HammerOverchargeLevel",0x7FFFFFFF);
    TakeInventory("EnviroDamageCount",0x7FFFFFFF);
    TakeInventory("EnviroDamageCooldown",0x7FFFFFFF);
    TakeInventory("StopTheBlock",0x7FFFFFFF);
    TakeInventory("BlockLife",0x7FFFFFFF);
    TakeInventory("GhostStepCooldown",0x7FFFFFFF);
    TakeInventory("DoubleTapCooldown",0x7FFFFFFF);
    TakeInventory("MidCombat",0x7FFFFFFF);
    TakeInventory("LegionCounter",0x7FFFFFFF);
    TakeInventory("LegionSpecialCounter",0x7FFFFFFF);
    TakeInventory("LegionStacked",0x7FFFFFFF);
    TakeInventory("SwordAttack",0x7FFFFFFF);

    TakeInventory("SlashingLikeABaka",1);
    TakeInventory("Kharon + Acacia A-22",1);
    TakeInventory("Kharon",1);
    TakeInventory("KamenRiderModeActivate",1);
    TakeInventory("UppercutTrigger",1);
    TakeInventory("UppercutUntrigger",1);
    TakeInventory("ZDoomTauntButton",1);
    TakeInventory("IsJungHaeLin",1);
    delay(1);
    GiveInventory("Kharon + Acacia A-22",1);
    GiveInventory("Kharon",1);
    GiveInventory("KamenRiderModeActivate",1);
    GiveInventory("UppercutTrigger",1);
    GiveInventory("UppercutUntrigger",1);
    GiveInventory("ZDoomTauntButton",1);
    GiveInventory("IsJungHaeLin",1);
}



// [13] Hoo, this shit.
// I have no clue how this works. As far as I'm concerned, it's dark magic.
// Do not touch, lest thy dark hand remain cursed. Many a night have I spent
// trying to optimize it, only to lay awake in bed, staring at the ceiling
// and watching compiler errors laugh down at me from the dark aeons split in
// the sky.
// I dread the day Zandronum finds alternative opportunities to handle clientside
// cvars, because on that day, there will be blood spilt.
// I pray it may not be mine.

//int array_custmischarg[PLAYERMAX];

function int WeebClientVars(void)
{
    int noannounce          = !!GetCVar("dst_cl_noannouncer");
    int nopan               = !!GetCVar("dst_cl_nopoints");
    int beepbeepbeep        = !!GetCVar("dst_cl_nobeeping");
    int autoswitch          = !!GetCVar("dst_cl_autoswitch");
    int recoilrules         = !!GetCVar("dst_cl_norecoil");

    return (noannounce << 4) + (nopan << 3) + (beepbeepbeep << 2) + (autoswitch << 1) + recoilrules;
}

script WEEB_ENTER_CLIENT ENTER clientside
{
    int execInt, oExecInt, execStr;
    int pln = PlayerNumber();

    while(1)
    {
        if (ConsolePlayerNumber() != PlayerNumber()) { Delay(1); continue; }

        if (CheckInventory("InIronMaiden") == 1)
        {
            // In Z& 2.0, if a player exits the level while in Iron Maiden, he gets a desynched-client "ghost" version of the Kharon and K+A22.
            TakeInventory("Kharon",1);
            TakeInventory("Kharon + Acacia A-22",1);
        }

        oExecInt = execInt;
        execInt = WeebClientVars();

        if (execInt != oExecInt)
        {
            if (!IsServer)
            {
                execStr = StrParam(s:"puke -", d:WEEB_PUKE, s:" ", d:execInt, s:" ", d:pln);
                ConsoleCommand(execStr);
            }
            else
            {
                ACS_ExecuteWithResult(WEEB_PUKE, execInt, pln);
            }
        }
        
        delay(1);
    }
}

script WEEB_PUKE (int values) net
{
    int pln = PlayerNumber();

    array_recoilrules[pln]     = values & 1;
    array_autoswitch[pln]      = values & 2;
    array_beepbeepbeep[pln]    = values & 4;
    array_nopan[pln]           = values & 8;
    array_noannounce[pln]      = values & 16;
}