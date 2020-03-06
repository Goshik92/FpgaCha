/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description:
*     The module implements a ChaCha20 algorithm.
*     It can be connected to Nios II or ARM processor
*     Using Avalon-MM interface
***********************************************/

localparam logic [4:0] ROUND_COUNT = 5'd20;
localparam StateIdx_t BCOUNT_IDX = StateIdx_t'(12);

// Data structures used in the module
typedef logic [31:0] Word_t;
typedef Word_t QState_t[4];
typedef Word_t State_t[16];
typedef logic [3:0] StateIdx_t;
typedef logic [$bits(State_t)-1:0] RawState_t;

// Rotates a word to the left by n positions
function automatic Word_t RotLeft(Word_t in, logic [4:0] n);
    RotLeft = in << n | in >> (~n + 1'b1); 
endfunction

// 
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
    input logic [5:0] csr_address,
    input logic [31:0] csr_writedata,
    output logic [31:0] csr_readdata,
    
    // Avalon-ST source interface for outputting the data
    output logic [511:0] st_data,
    output logic st_valid,
    input logic st_ready
);

    // Round being computed currently
    logic [4:0] roundCounter;
    
    // OTP being computed currently
    logic [4:0] padCounter;
    
    // ChaCha20 states
    State_t initState, state;
    
    // not actually a register and is used in a MUX
    State_t roundSrc, roundResult;
    
    // Assign random constant to probe the module 
    assign csr_readdata = 32'hfb7e03d9; 
    
    // Choose source for a round (init state or current state)
    assign roundSrc = roundCounter == 1'b0 ? initState : state;
                
    // Get round result
    // roundCounter[0] is used to choose even or odd round function
    assign roundResult = roundCounter[0] ? OddRound(roundSrc) : EvenRound(roundSrc);
    
    //
    assign st_data = ToRawState(state);

    //
    task ResetRound();
        padCounter <= 1'b0;
        roundCounter <= 1'b0;
        st_valid <= 1'b0;
    endtask
    
    //
    task FirstRound();
        // Set round counter to the top value
        roundCounter <= ROUND_COUNT - 1'b1;
        
        // Save first round output
        state <= roundResult;
        
        // Increment block count to compute next pad
        // Has no effect for the current pad
        initState[BCOUNT_IDX] <= initState[BCOUNT_IDX] + 1'b1;
        
        //
        st_valid <= 1'b0;
    endtask
    
    //
    task NextRound();
        // One more round has been processed
        roundCounter <= roundCounter - 1'b1;
                    
        // Save current round output
        state <= roundResult;
        
        //
        st_valid <= roundCounter == 1'b1;
    endtask

    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) ResetRound();
        
        // Non-reset logic
        else begin
            // Avalon write operation
            if (csr_write) casez(csr_address)
                // Fill initial state
                6'b00????: begin
                    initState[csr_address[3:0]] <= csr_writedata;
                    ResetRound();
                end
                
                // If CONTROL register is written, first computation
                6'b100000: begin
                    padCounter <= csr_writedata[4:0];
                    FirstRound();
                end
            endcase

            // No write operations from Avalon
            else begin            
                // Do next round
                if (roundCounter != 1'b0) NextRound();
                
                // Last round logic
                else begin
                    // Continue only if the the stream sink can accept data.
                    // Otherwise stall (roundCounter does not change)
                    if (st_ready && padCounter != 1'b0) begin
                        // One more pad has been computed
                        padCounter <= padCounter - 1'b1;
                        
                        // Start new pad computation
                        FirstRound();
                    end
                end
            end
        end
    end
endmodule