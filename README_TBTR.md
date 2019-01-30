# OpenTTD TBTR

## Note about version 2.0

This patch was developed some years ago and I left it alone for quite a while.
Version 2.0 is an attempt to clean up the UI and implementation of the game.

*The major improvment should be that it is tested in network games to make sure that it doesn't cause
any desyncs.*

The UI consists now of a single window including a list of engines to add to
the templates. The original UI was very cluttered and clumsy, imho.

(Due to removing a big ugly hack in the implementation...)
### The creation of templates has changed in a way that it is not as powerful as it used to be. You can:
* Clone a template from an existing train (including its refit)
* Create a new template by Appending a new engine without selecting any existing template from the list
* Append more engines to the selected template
* Remove the last engine from the selected template
* Delete a template

### The 3 Template usage options remain:
* Use or don't use existing vehicles from the depot
* Keep or sell remainders after the template replacement process
* Apply the template's refit settings to the treated train

### Some options have not been implemented in 2.0 so far
* The possibility to set a template's refit/cargo type
* The option to modify all templates at once by replacing an engine of a specific type in all
  templates at once
* Drag&drop editing of templates

## Hotkey: CTRL+E opens the TBTR UI

My plan is to implement missing features and to keep this patch up-to-date with the core game while
providing Linux + Windows binaries for the latest stable release and the currently nightly build of
the core game.

Here are now the original notes which should still be valid (by ~90% ;))

## What does it do ?

In short:

* it provides an advanced auto-replace mechanism for trains and the various subtypes
* it allows the player to pre-define 'templates' of how a train should look like a vehicle group is
  then set up to replace all of its trains to look exactly like the defined template on the next
  depot visit of any train in that group, it will get changed to match the template, buying new
  vehicles, moving old ones around as necessary
* remaining parts of a train after the replacement step may either be kept in the depot for later
  reuse or may be sold automatically

## Why use it ?

The motivation of this patch is to reduce micro-management in larger scale games. There are multiple
scenarios where I don't think the current auto-replace feature is sufficient:

* You are running a line for any good which you set up nicely to support the needed amount of
  transported cargo. At some later point in the game, new wagons become available that have
  significantly increased capacities. You can either replace them all via autoreplacement but then
  lack efficiency as you wouldn't have needed as many due to the increased capacity of each wagon.
  Or you would need to change all trains by hand to contain a smaller number of the newer, better
  wagons.

* You are running heavy trains that require multiple engines to be efficient. When later in the game
  newer and stronger engines become available it might be sufficient to use a single engine for each
  train instead of multiple ones due to higher power of latest technologies. And you now want to
  change up your trains to use 1 new engine instead of 2 or more old ones.

Both scenarios (and possibly more) are really only solvable with a lot of manual
sending-trains-to-depot-and-modifying-them-by-hand. Both scenarios occurred to me especially in
larger scale games with custom newgrf sets where the change in capacity and power of vehicles over
time can be severe.  With template based train replacement, you can specify exactly what you want
your trains to be composed of - and what refit they should use - and have them changed up in the
depot automatically.

## How to use it ?

### There are a couple of notes about its usage:

When opening a group window (for trains, please), you find a new drop-down entry for managing the
group which reads Template Replacement and will open up the new gui element.  Select a group from
the top list and a template-train from the bottom list, then press 'start replacing' which will
trigger the replacement function whenever any train of the selected group will enter a depot the
next time.

Autoreplacement as you know it, is of course still available for use The functionality is tightly
tied to using groups, i.e. all automatic replacements are set up on a per-group basis, otherwise it
would be very hard to specify which train should be composed in which way.

The template replacement function will _always_ attempt to reuse as much of the treated train as
possible so that as few vehicles as possible will be bought the order in which vehicles of a
template are defined is essential to the replacement, so we will not only make sure that the new
train includes every vehicle from the template, but also in the correct order to prevent messed up
trains roaming the game world ;)

### Configurable behaviour:

A train that is getting template replaced in any depot, might be using vehicles that are idle inside
the depot instead of buying new ones, so you can leave old engines or wagons in depots for later
re-composition of other trains.

After a train got changed by template-replacement, its old engines and wagons which are not part of
the consist anymore, can optionally remain inside the depot in a neutral and idle state for later
usage, and are not sold automatically if the option is set so.

I will add suggestions or criticism to a todo-list in this thread if you want to help me by providing some.
