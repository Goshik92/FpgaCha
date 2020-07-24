/*******************************************************************************
 * Copyright 2020 Igor Semenov (goshik92@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *******************************************************************************/

// Data structures used in the module
typedef logic [31:0] Word_t;
typedef Word_t QState_t[4];
typedef Word_t State_t[16];
typedef logic [3:0] StateIdx_t;
typedef logic [$bits(State_t)-1:0] RawState_t;
typedef logic [4:0] RoundCounter_t;

localparam RoundCounter_t MAX_ROUND_COUNT = 5'd19;
localparam StateIdx_t BCOUNT_IDX = StateIdx_t'(12);

// Rotates a word to the left by n positions
function automatic Word_t RotLeft(Word_t in, logic [4:0] n);
    RotLeft = in << n | in >> (~n + 1'b1); 
endfunction

// Converts a word array into a bit array
function automatic RawState_t ToRawState(State_t s);
    for(int i = 0; i < $size(State_t); i++) begin
        ToRawState[i * $bits(Word_t) +: $bits(Word_t)] = s[i];
    end
endfunction

// Performs one quarter round of ChaCha20
function automatic QState_t QRound(input QState_t in);
    Word_t a = in[0] + in[1];
    Word_t d = RotLeft((in[3] ^ a), 5'd16);
    Word_t c = in[2] + d;
    Word_t b = RotLeft((in[1] ^ c), 5'd12);
    QRound[0] = a + b;
    QRound[3] = RotLeft((d ^ QRound[0]), 5'd8);
    QRound[2] = c + QRound[3];
    QRound[1] = RotLeft((b ^ QRound[2]), 5'd7);
endfunction

// Performs one quarter round on State based on supplied offsets
function automatic State_t QRoundIdx(input State_t s, input StateIdx_t idx[4]);
    QState_t out = QRound('{s[idx[0]], s[idx[1]], s[idx[2]], s[idx[3]]});
    QRoundIdx = s;
    for(int i = 0; i < $size(QState_t); i++) QRoundIdx[idx[i]] = out[i];
endfunction

// Does one even round
function automatic State_t EvenRound(input State_t s);
    s = QRoundIdx(s, '{0, 4,  8, 12});
    s = QRoundIdx(s, '{1, 5,  9, 13});
    s = QRoundIdx(s, '{2, 6, 10, 14});
    s = QRoundIdx(s, '{3, 7, 11, 15});
    EvenRound = s;
endfunction

// Does one odd round
function automatic State_t OddRound(input State_t s);
    s = QRoundIdx(s, '{0, 5, 10, 15});
    s = QRoundIdx(s, '{1, 6, 11, 12});
    s = QRoundIdx(s, '{2, 7,  8, 13});
    s = QRoundIdx(s, '{3, 4,  9, 14});
    OddRound = s;
endfunction

module ChaCha20(
    input logic clock,
    input logic reset,
    
    // Avalon-MM slave interface for configuring the module
    input logic csr_write,
    input logic csr_read,
    input logic [4:0] csr_address,
    input Word_t csr_writedata,
    output Word_t csr_readdata,
    
    // Avalon-ST source interface for outputting the data
    output logic [511:0] st_data,
    output logic st_valid,
    input logic st_ready
);

    // Round being computed currently
    RoundCounter_t roundCounter;
    
    // Number of OTP being computed currently
    Word_t padCounter;
    
    // ChaCha20 states
    State_t initState, state;
    
    // not actually a register and is used in a MUX
    State_t roundResult, stateSrc;
        
    //
    assign stateSrc = roundCounter == 1'b0 ? initState : state;
         
    // Get round result
    // roundCounter[0] is used to choose even or odd round function
    assign roundResult = roundCounter[0] ? OddRound(stateSrc) : EvenRound(stateSrc);
    
    // Connect out data directly to the state
    assign st_data = ToRawState(state);

    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) begin
            st_valid <= 1'b0;
            roundCounter <= 1'b0;
            padCounter <= 1'b0;
        end
        
        // Non-reset logic
        else begin
            // Avalon read operation
            if (csr_read) casez(csr_address)
                5'b0????: csr_readdata <= initState[csr_address[3:0]];
                5'b10000: csr_readdata <= padCounter;
                5'b10001: csr_readdata <= roundCounter;
                // random constant to probe the module 
                default: csr_readdata <= 32'hfb7e03d9; 
            endcase
        
            // Avalon write operation
            if (csr_write) casez(csr_address)
                // Words of initial state
                5'b0????: begin
                    initState[csr_address[3:0]] <= csr_writedata;
                end
                
                // If padCounter register is written
                5'b10000: begin
                    padCounter <= csr_writedata;
                    roundCounter <= 1'b0;
                end
            endcase

            // If there is work to do
            else if (padCounter != 1'b0) begin  
                // If we can change output data
                if (st_ready || !st_valid) begin
                    // Save round result
                    state <= roundResult;
                    
                    // First round logic
                    if (roundCounter == 1'b0) begin
                        // One more round has been processed
                        roundCounter <= roundCounter + 1'b1;

                        // Increment block count to compute next pad
                        // Has no effect on the current pad
                        initState[BCOUNT_IDX] <= initState[BCOUNT_IDX] + 1'b1;
                        
                        // Output data is invalid now
                        st_valid <= 1'b0;
                    end
                    
                    // Regular round logic
                    else if (roundCounter != MAX_ROUND_COUNT) begin
                        // One more round has been processed
                        roundCounter <= roundCounter + 1'b1;
                    end
                    
                    // Last round logic
                    else begin
                        // Start new pad computation
                        roundCounter <= 1'b0;
                        
                        // One more pad has been computed
                        padCounter <= padCounter - 1'b1;
                        
                        // Output data is valid now
                        st_valid <= 1'b1;
                    end
                end
            end
            
            else if (st_ready) st_valid <= 1'b0;
        end
    end
endmodule