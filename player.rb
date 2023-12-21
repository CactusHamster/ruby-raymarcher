require_relative "ext/window_utils/window_utils"
include GLFW
class Player
    MAX_SPEED = 1000.0
    MOVE_SPEED_PER_SECOND = 1000.0
    JUMP_SPEED = 240.0
    GRAVITY = 14.0
    FLYING = false
    attr_accessor :"position", :"rotation"
    def initialize (context)
        @context = context
        @rotation = [0.0, 0.0]
        @forces   = [0.0, 0.0, 0.0]
        @velocity = [0.0, 0.0, 0.0]
        @position = [0, 0.0, -360.0]
    end
    
    def add_rotation (pitch, yaw)
        # Keep pitch between -PI and PI to prevent gimbal lock.
        @rotation[0] += [pitch, -Math::PI / 2, Math::PI / 2].sort[1]
        @rotation[1] += yaw
    end
    def jump ()
        @velocity[1] += JUMP_SPEED if on_ground?()
    end
    def apply_gravity()
        @forces[1] -= GRAVITY if @velocity[1] > GRAVITY * -5 and not FLYING
    end
    def apply_friction ()
        # If we're flying, we get no friction.
        if FLYING then
            @velocity[0] *= 0.9
            @velocity[1] *= 0.9
            @velocity[2] *= 0.9
        else
            # No friction while falling.
            if on_ground?()
                @forces[0] -= 0.1 * @velocity[0]
                @forces[2] -= 0.1 * @velocity[2]
                # @velocity[0] *= 0.9
                # @velocity[2] *= 0.9
            end
        end
    end

    # Apply external forces.
    def update_forces ()
        apply_gravity() if not on_ground?()
        apply_friction()
    end
    # Apply forces to velocity.
    def update_velocity ()
        @velocity[0] += @forces[0]
        @velocity[1] += @forces[1]
        @velocity[2] += @forces[2]
        @forces[0] = 0
        @forces[1] = 0
        @forces[2] = 0
    end
    # Apply velocity to position.
    def update_position (elapsed_time)
        # Update the player's position based on velocity
        @position[0] += @velocity[0] * elapsed_time
        @position[1] += @velocity[1] * elapsed_time
        @position[2] += @velocity[2] * elapsed_time
    
        # Clamp the player to the ground to prevent falling through it
        @position[1] = [@position[1], Context::GROUND_HEIGHT].max
    end

    def update(elapsed_time, mouse_movement)
        process_mouse_movement(elapsed_time, *mouse_movement)
        update_forces()
        process_held_keys(elapsed_time) # Avoid friction acting on first movement update.
        update_velocity()
        is_moving = (@velocity[0]*@velocity[0]) + (@velocity[1] * @velocity[1]) + (@velocity[2] * @velocity[2]) != 0
        update_position(elapsed_time)
        @context.send_camera_uniform() if is_moving
    end
    
    def process_held_keys(elapsed_time)

        # Secret speed boost
        if @context.key_held?(:"KP_ENTER") then
            # Increase speed by order of 1000.
            # MOVE_SPEED_PER_SECOND *= 1000
        elsif @context.key_held?(:"RIGHT_SHIFT") then
            # Decrease speed by order of 1000.
            # MOVE_SPEED_PER_SECOND /= 1000
        end

        # Tell direction we're moving in.
        translate_left_right = case
            when @context.key_held?(:"D") then 1
            when @context.key_held?(:"A") then -1
            else 0
        end
        translate_depth = case
            when @context.key_held?(:"W") then 1
            when @context.key_held?(:"S") then -1
            else 0
        end
        
        update_player = false
        if translate_depth != 0
            magnitude = translate_depth * MOVE_SPEED_PER_SECOND * elapsed_time
            angle = @rotation[1]
            @velocity[0] += magnitude * Math.sin(angle)
            @velocity[2] += magnitude * Math.cos(angle)
            update_player = true
        end
        if translate_left_right != 0
            magnitude = translate_left_right * MOVE_SPEED_PER_SECOND * elapsed_time
            angle = @rotation[1]
            @velocity[0] += magnitude * Math.cos(angle)
            @velocity[2] -= magnitude * Math.sin(angle)
            update_player = true
        end
        
        # Extra check for up/down movement if we can fly.
        if FLYING
            translate_up_down = case
                when @context.key_held?(:"SPACE") then 1
                when @context.key_held?(:"LEFT_SHIFT") then -1
                else 0
            end
            if translate_up_down != 0
                @velocity[1] += translate_up_down * MOVE_SPEED_PER_SECOND * elapsed_time
                update_player = true
            end
        else
            if on_ground? and @context.key_held?(:"SPACE") then
                @velocity[1] += JUMP_SPEED
            end
            if on_ground? and @context.key_held?(:"LEFT_SHIFT") then
                @velocity[0] *= 0.2
                @velocity[2] *= 0.2
            end
        end
        
        # Return true if we should resend the position vector.
        return update_player
    end

    def process_mouse_movement(elapsed_time, dx, dy)
        return nil if dx * dx + dy * dy == 0
        dx = dx * 0.05 * elapsed_time
        dy = dy * 0.05 * elapsed_time
        add_rotation(dy, dx)
        @context.send_rotation_uniform()
    end

    # Player is on or close to the ground
    def on_ground?
        @position[1] <= Context::GROUND_HEIGHT + 0.01
    end
end
