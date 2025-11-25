"""
Editor Bridge - Interfaces with Unreal Engine 5 Editor via Python API

This bridge provides two modes of operation:
1. Direct Mode: When running inside UE Python environment, uses unreal module directly
2. Subprocess Mode: When running outside UE, uses unreal_editor_python_scripts or RPC

The bridge exposes a unified interface for all editor operations.
"""

import asyncio
import json
import logging
import subprocess
import sys
from pathlib import Path
from typing import Any, Optional
from dataclasses import dataclass

logger = logging.getLogger("hkt_mcp.editor_bridge")


@dataclass
class AssetInfo:
    """Asset information structure"""
    asset_path: str
    asset_name: str
    asset_class: str
    package_path: str


@dataclass
class ActorInfo:
    """Actor information structure"""
    actor_name: str
    actor_label: str
    actor_class: str
    location: tuple[float, float, float]
    rotation: tuple[float, float, float]
    scale: tuple[float, float, float]
    actor_guid: str


class EditorBridge:
    """
    Bridge for Unreal Engine 5 Editor operations
    
    Supports both direct unreal module access and external subprocess/RPC calls.
    """
    
    def __init__(self):
        self._unreal_module: Any = None
        self._subsystem: Any = None
        self._is_direct_mode = False
        self._project_path: Optional[Path] = None
        
        # Try to detect project path from environment
        import os
        project_path = os.environ.get("UE_PROJECT_PATH")
        if project_path:
            self._project_path = Path(project_path)
            logger.info(f"Project path set to: {self._project_path}")
    
    def set_unreal_module(self, unreal_module: Any) -> None:
        """Set the unreal module for direct mode"""
        self._unreal_module = unreal_module
        self._is_direct_mode = True
        
        # Get the editor subsystem
        try:
            self._subsystem = unreal_module.get_editor_subsystem(
                unreal_module.HktMcpEditorSubsystem
            )
            logger.info("Direct mode enabled with HktMcpEditorSubsystem")
        except Exception as e:
            logger.warning(f"Could not get HktMcpEditorSubsystem: {e}")
            # Will use direct unreal API calls instead
    
    @property
    def is_direct_mode(self) -> bool:
        """Check if running in direct mode"""
        return self._is_direct_mode
    
    # ==================== Project Info ====================
    
    async def get_project_info(self) -> str:
        """Get project information"""
        if self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                project_dir = unreal.Paths.project_dir()
                project_name = unreal.Paths.get_base_filename(unreal.Paths.get_project_file_path())
                engine_version = unreal.SystemLibrary.get_engine_version()
                
                return json.dumps({
                    "project_name": project_name,
                    "project_dir": project_dir,
                    "engine_version": engine_version,
                    "mode": "direct"
                })
            except Exception as e:
                logger.error(f"Failed to get project info: {e}")
                return json.dumps({"error": str(e)})
        else:
            return json.dumps({
                "project_path": str(self._project_path) if self._project_path else "unknown",
                "mode": "external"
            })
    
    async def get_current_level_info(self) -> str:
        """Get current level information"""
        if self._subsystem:
            try:
                return self._subsystem.get_current_level_info()
            except Exception as e:
                return json.dumps({"error": str(e)})
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                world = unreal.EditorLevelLibrary.get_editor_world()
                if world:
                    return json.dumps({
                        "world_name": world.get_name(),
                        "map_name": world.get_map_name(),
                    })
            except Exception as e:
                return json.dumps({"error": str(e)})
        
        return json.dumps({"error": "Not connected to editor"})
    
    # ==================== Asset Operations ====================
    
    async def list_assets(self, path: str, class_filter: str = "") -> list[dict]:
        """List assets in specified path"""
        if self._subsystem:
            try:
                assets = self._subsystem.list_assets(path, class_filter)
                return [
                    {
                        "asset_path": a.asset_path,
                        "asset_name": a.asset_name,
                        "asset_class": a.asset_class,
                        "package_path": a.package_path
                    }
                    for a in assets
                ]
            except Exception as e:
                logger.error(f"Failed to list assets: {e}")
                return []
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
                
                assets = asset_registry.get_assets_by_path(path, recursive=True)
                result = []
                for asset in assets:
                    if class_filter and class_filter not in str(asset.asset_class_path):
                        continue
                    result.append({
                        "asset_path": str(asset.get_full_name()),
                        "asset_name": str(asset.asset_name),
                        "asset_class": str(asset.asset_class_path),
                        "package_path": str(asset.package_path)
                    })
                return result
            except Exception as e:
                logger.error(f"Failed to list assets: {e}")
                return []
        
        return []
    
    async def get_asset_details(self, asset_path: str) -> str:
        """Get detailed asset information"""
        if self._subsystem:
            return self._subsystem.get_asset_details(asset_path)
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                asset = unreal.EditorAssetLibrary.load_asset(asset_path)
                if asset:
                    return json.dumps({
                        "name": asset.get_name(),
                        "class": asset.get_class().get_name(),
                        "path": asset_path
                    })
            except Exception as e:
                return json.dumps({"error": str(e)})
        
        return json.dumps({"error": "Not connected to editor"})
    
    async def search_assets(self, query: str, class_filter: str = "") -> list[dict]:
        """Search assets by name"""
        if self._subsystem:
            try:
                assets = self._subsystem.search_assets(query, class_filter)
                return [
                    {
                        "asset_path": a.asset_path,
                        "asset_name": a.asset_name,
                        "asset_class": a.asset_class,
                        "package_path": a.package_path
                    }
                    for a in assets
                ]
            except Exception as e:
                logger.error(f"Failed to search assets: {e}")
                return []
        
        # Fallback: list all and filter
        all_assets = await self.list_assets("/Game", class_filter)
        return [a for a in all_assets if query.lower() in a["asset_name"].lower()]
    
    async def modify_asset_property(self, asset_path: str, property_name: str, new_value: str) -> bool:
        """Modify an asset property"""
        if self._subsystem:
            return self._subsystem.modify_asset_property(asset_path, property_name, new_value)
        
        logger.warning("modify_asset_property not available without subsystem")
        return False
    
    # ==================== Level Operations ====================
    
    async def list_actors(self, class_filter: str = "") -> list[dict]:
        """List actors in current level"""
        if self._subsystem:
            try:
                actors = self._subsystem.list_actors(class_filter)
                return [
                    {
                        "actor_name": a.actor_name,
                        "actor_label": a.actor_label,
                        "actor_class": a.actor_class,
                        "location": {"x": a.location.x, "y": a.location.y, "z": a.location.z},
                        "rotation": {"pitch": a.rotation.pitch, "yaw": a.rotation.yaw, "roll": a.rotation.roll},
                        "scale": {"x": a.scale.x, "y": a.scale.y, "z": a.scale.z},
                        "actor_guid": a.actor_guid
                    }
                    for a in actors
                ]
            except Exception as e:
                logger.error(f"Failed to list actors: {e}")
                return []
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                actors = unreal.EditorLevelLibrary.get_all_level_actors()
                result = []
                for actor in actors:
                    if class_filter and class_filter not in actor.get_class().get_name():
                        continue
                    loc = actor.get_actor_location()
                    rot = actor.get_actor_rotation()
                    scale = actor.get_actor_scale3d()
                    result.append({
                        "actor_name": actor.get_name(),
                        "actor_label": actor.get_actor_label(),
                        "actor_class": actor.get_class().get_name(),
                        "location": {"x": loc.x, "y": loc.y, "z": loc.z},
                        "rotation": {"pitch": rot.pitch, "yaw": rot.yaw, "roll": rot.roll},
                        "scale": {"x": scale.x, "y": scale.y, "z": scale.z},
                        "actor_guid": ""
                    })
                return result
            except Exception as e:
                logger.error(f"Failed to list actors: {e}")
                return []
        
        return []
    
    async def spawn_actor(
        self,
        blueprint_path: str,
        location: tuple[float, float, float],
        rotation: tuple[float, float, float] = (0, 0, 0),
        label: str = ""
    ) -> str:
        """Spawn an actor from a blueprint"""
        if self._subsystem:
            try:
                unreal = self._unreal_module
                loc = unreal.Vector(location[0], location[1], location[2])
                rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
                return self._subsystem.spawn_actor(blueprint_path, loc, rot, label)
            except Exception as e:
                logger.error(f"Failed to spawn actor: {e}")
                return ""
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                bp = unreal.EditorAssetLibrary.load_asset(blueprint_path)
                if bp:
                    loc = unreal.Vector(location[0], location[1], location[2])
                    rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
                    actor = unreal.EditorLevelLibrary.spawn_actor_from_object(bp, loc, rot)
                    if actor and label:
                        actor.set_actor_label(label)
                    return actor.get_name() if actor else ""
            except Exception as e:
                logger.error(f"Failed to spawn actor: {e}")
                return ""
        
        return ""
    
    async def modify_actor_transform(
        self,
        actor_name: str,
        location: Optional[tuple[float, float, float]] = None,
        rotation: Optional[tuple[float, float, float]] = None,
        scale: Optional[tuple[float, float, float]] = None
    ) -> bool:
        """Modify actor transform"""
        if self._subsystem and self._unreal_module:
            try:
                unreal = self._unreal_module
                
                # Get current values if not provided
                actors = await self.list_actors()
                actor_data = next((a for a in actors if a["actor_name"] == actor_name or a["actor_label"] == actor_name), None)
                if not actor_data:
                    return False
                
                current_loc = actor_data["location"]
                current_rot = actor_data["rotation"]
                current_scale = actor_data["scale"]
                
                new_loc = unreal.Vector(
                    location[0] if location else current_loc["x"],
                    location[1] if location else current_loc["y"],
                    location[2] if location else current_loc["z"]
                )
                new_rot = unreal.Rotator(
                    rotation[0] if rotation else current_rot["pitch"],
                    rotation[1] if rotation else current_rot["yaw"],
                    rotation[2] if rotation else current_rot["roll"]
                )
                new_scale = unreal.Vector(
                    scale[0] if scale else current_scale["x"],
                    scale[1] if scale else current_scale["y"],
                    scale[2] if scale else current_scale["z"]
                )
                
                return self._subsystem.modify_actor_transform(actor_name, new_loc, new_rot, new_scale)
            except Exception as e:
                logger.error(f"Failed to modify actor: {e}")
                return False
        
        return False
    
    async def delete_actor(self, actor_name: str) -> bool:
        """Delete an actor"""
        if self._subsystem:
            return self._subsystem.delete_actor(actor_name)
        elif self._is_direct_mode and self._unreal_module:
            try:
                unreal = self._unreal_module
                actors = unreal.EditorLevelLibrary.get_all_level_actors()
                for actor in actors:
                    if actor.get_name() == actor_name or actor.get_actor_label() == actor_name:
                        return unreal.EditorLevelLibrary.destroy_actor(actor)
            except Exception as e:
                logger.error(f"Failed to delete actor: {e}")
        
        return False
    
    async def select_actor(self, actor_name: str) -> bool:
        """Select an actor in the editor"""
        if self._subsystem:
            return self._subsystem.select_actor(actor_name)
        
        return False
    
    # ==================== Query Operations ====================
    
    async def search_classes(self, query: str, blueprint_only: bool = False) -> list[str]:
        """Search for classes by name"""
        if self._subsystem:
            return list(self._subsystem.search_classes(query, blueprint_only))
        
        return []
    
    async def get_class_properties(self, class_name: str) -> list[dict]:
        """Get properties of a class"""
        if self._subsystem:
            try:
                props = self._subsystem.get_class_properties(class_name)
                return [
                    {
                        "property_name": p.property_name,
                        "property_type": p.property_type,
                        "is_editable": p.b_is_editable
                    }
                    for p in props
                ]
            except Exception as e:
                logger.error(f"Failed to get class properties: {e}")
        
        return []
    
    async def get_project_structure(self, root_path: str = "/Game") -> str:
        """Get project folder structure"""
        if self._subsystem:
            return self._subsystem.get_project_structure(root_path)
        
        return json.dumps({"root": root_path, "folders": [], "mode": "external"})
    
    # ==================== Editor Control ====================
    
    async def start_pie(self) -> bool:
        """Start Play In Editor"""
        if self._subsystem:
            return self._subsystem.start_pie()
        
        return False
    
    async def stop_pie(self) -> bool:
        """Stop Play In Editor"""
        if self._subsystem:
            return self._subsystem.stop_pie()
        
        return False
    
    async def is_pie_running(self) -> bool:
        """Check if PIE is running"""
        if self._subsystem:
            return self._subsystem.is_pie_running()
        
        return False
    
    async def execute_command(self, command: str) -> None:
        """Execute editor console command"""
        if self._subsystem:
            self._subsystem.execute_editor_command(command)
    
    # ==================== Viewport Operations ====================
    
    async def get_viewport_camera(self) -> dict:
        """Get viewport camera transform"""
        if self._subsystem:
            try:
                location = self._unreal_module.Vector()
                rotation = self._unreal_module.Rotator()
                if self._subsystem.get_viewport_camera_transform(location, rotation):
                    return {
                        "location": {"x": location.x, "y": location.y, "z": location.z},
                        "rotation": {"pitch": rotation.pitch, "yaw": rotation.yaw, "roll": rotation.roll}
                    }
            except Exception as e:
                logger.error(f"Failed to get viewport camera: {e}")
        
        return {"error": "Failed to get viewport camera"}
    
    async def set_viewport_camera(
        self,
        location: tuple[float, float, float],
        rotation: tuple[float, float, float]
    ) -> bool:
        """Set viewport camera transform"""
        if self._subsystem and self._unreal_module:
            try:
                unreal = self._unreal_module
                loc = unreal.Vector(location[0], location[1], location[2])
                rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
                return self._subsystem.set_viewport_camera_transform(loc, rot)
            except Exception as e:
                logger.error(f"Failed to set viewport camera: {e}")
        
        return False
    
    async def show_notification(self, message: str, duration: float = 3.0) -> None:
        """Show editor notification"""
        if self._subsystem:
            self._subsystem.show_notification(message, duration)

