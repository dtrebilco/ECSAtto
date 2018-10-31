#pragma once

#include "GameContext.h"

/// \brief This file contains helper methods that are safe to use, but may be inefficient
namespace safe
{
  /// \brief Get the local position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to get the position for
  /// \return Returns the local position if it exists or a zero vector if not
  static vec3 GetLocalPosition(GameContext i_c, EntityID i_entity);

  /// \brief Set the local position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to set the position for
  /// \param i_position The new position to set
  static void SetLocalPosition(GameContext i_c, EntityID i_entity, const vec3& i_position);
   
  /// \brief Get the global position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to get the position for
  /// \return Returns the global position if it exists or a zero vector if not
  static vec3 GetGlobalPosition(GameContext i_c, EntityID i_entity);

  /// \brief Set the global position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to set the position for
  /// \param i_position The new position to set
  static void SetGlobalPosition(GameContext i_c, EntityID i_entity, const vec3& i_position);

}




